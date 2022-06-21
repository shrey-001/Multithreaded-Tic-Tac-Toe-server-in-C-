
#include <iostream>
#include <algorithm>
#include <list>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <bits/local_lim.h>
//#include "local_sock.h"
#include "player.h"
#include "game.h"
#include "status.cpp"
//#include "status_codes.h"
//#include "sockets.h"

//#include "player.cpp"
//#include "game.cpp"


#define SERVERPORT 8080
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100// number of waiting collenction the server allows
#define THREAD_POOL_SIZE 20 // number of threads in the server

// pthread_t thread_pool[THREAD_POOL_SIZE] ; // fixed number of threads

pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER ; // mutex locks
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER ;


using std::cout;
using std::endl;
using std::list;
using std::find;
using std::remove_if;
using std::find_if;

list<TTTGame *> game_list;



typedef struct sockaddr_in SA_IN ;
typedef struct sockaddr SA ;

void * handle_connection(void * p_clinet_soccket) ;
int check(int exp , const char *msg);
bool ProcessCommand(char buffer[], Player & player, bool & client_connected);
void DisconnectPlayer(const Player & player);
void createJoinGame(Player & player, string game_name);
void listGames(Player & player);


int main(int argc , char **argv){
    int server_socket, addr_size;
    SA_IN server_addr;

    check((server_socket = socket(AF_INET , SOCK_STREAM , 0)),
            "Failed to create socket") ;
    
    server_addr.sin_family = AF_INET ;
    server_addr.sin_addr.s_addr = INADDR_ANY ;
    server_addr.sin_port = htons(SERVERPORT) ;
    
    
    int opt=1;
    if (setsockopt(server_socket, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))){
        perror("setsocket opt error");
        exit(1);
    }

    check((bind(server_socket, (SA*)&server_addr , sizeof(server_addr))),
            "Bind Failed!") ;
    check(listen(server_socket, SERVER_BACKLOG),
            "Listen Failed!") ;
    
    int client_socket;
    SA_IN client_addr;
    
    while(true){
        printf("Waiting for connection...\n") ;
        addr_size = sizeof(SA_IN) ;
        check(client_socket = accept(server_socket , (SA*)&client_addr, (socklen_t*)&addr_size),
                "accept failed!!") ;
        printf("Connected\n") ;
    
        pthread_t threadid;
        pthread_create(&threadid, NULL, handle_connection, (void *)&client_socket);
        
    }

    return 0 ;

}
void * handle_connection(void * p_clinet_socket) {
    int client_sock = *(int *)p_clinet_socket;
    
    char buffer[BUFSIZE];
    bool client_connected = true;
    
    char temp = '\0';
    int row = 0, col = 0;
    int i = 0;

    // Create the player
    Player player(client_sock);

    //cout<<player.getMode();

    // Always handle the client
    while(client_connected)
    {
        // Process commands or pass game data
        if(!player.getMode())
        {
            // Read a line of text or until the buffer is full
            for(i = 0; (i < (BUFSIZE - 1)) && temp != '\n' && client_connected; ++i)
            {
                // Receive a single character and make sure the client is still connected
                if(read(client_sock, &temp, 1) == 0)
                    client_connected = false;
                else
                    buffer[i] = temp;
            }

            // Reset temp so we don't get an infinite loop
            temp = '\0';
            buffer[i] = '\0';
            buffer[i - 1] = '\0';
            cout << "Received command \"" << buffer << "\" from " << player.getName() << endl;
            buffer[i - 1] = '\n';


            // If there's an invalid command, tell the client
            if(!ProcessCommand(buffer, player, client_connected))
                continue;
                //SendStatus(player.GetSocket(), INVALID_CMD);
        }
        else{
            
            pthread_mutex_lock(&games_lock);
            auto game = find_if(game_list.begin(), game_list.end(), [player] (TTTGame * game) { return game->HasPlayer(player); });
            auto end = game_list.end();
            pthread_mutex_unlock(&games_lock);

            // Something horrible has gone wrong
            if(game == end)
                cout << "Somehow Player " << player.getName() << " isn't a part of a game but is INGAME" << endl;
            else
            {
                continue;
                /*StatusCode status;
                client_connected = ReceiveStatus(player.getSocket(), &status);

                // If the player is still connected, then perform the move
                if(client_connected)
                {
                    switch(status)
                    {
                        case MOVE:
                            // Pass the row and column right along
                            ReceiveInt(player.getSocket(), &row);
                            ReceiveInt(player.getSocket(), &col);
                            cout << "Received moved from " << player.getName()
                                 << ": row=" << row << ", col=" << col << endl;

                            SendStatus((*game)->GetOtherPlayer(player).getSocket(), MOVE);
                            SendInt((*game)->GetOtherPlayer(player).getSocket(), row);
                            client_connected = SendInt((*game)->GetOtherPlayer(player).getSocket(), col);
                            cout << "Sent move to " << (*game)->GetOtherPlayer(player).getName() << endl;

                            break;
                        case WIN:
                            cout << player.getName() << " won a game against " << (*game)->GetOtherPlayer(player).getName() << endl;
                            client_connected = false;
                            break;
                        case DRAW:
                            cout << player.getName() << " tied against " << (*game)->GetOtherPlayer(player).getName() << endl;
                            client_connected = false;
                            break;
                        default:
                            client_connected = SendStatus(player.getSocket(), INVALID_CMD);
                    }
                }*/
            }
        }
    }

    cout << "Player \"" << player.getName() << "\" has disconnected" << endl;
    DisconnectPlayer(player);
    close(client_sock);

    return (void *)0;
}
//bool ProcessCommand(char buffer[], Player & player, bool & client_connected){
bool ProcessCommand(char buffer[], Player & player, bool & client_connected){
    char s_command[BUFSIZE];
    char s_arg[BUFSIZE];
    string command, arg;
    bool valid_cmd = true;

    // Convert the input command into separate command and argument
    int num = sscanf(buffer, "%s %s", s_command, s_arg);
    command = s_command;
    arg = s_arg;

    // Scanf needs to have captured either one or two strings
    if(command == "join" && num == 2){
        createJoinGame(player, arg);
    }
    else if (command == "register" && num == 2){
        // Set the player's name
        player.setName(arg);
        //SendStatus(player.GetSocket(), REGISTERED);
        cout << "Registered player name \"" << arg << "\"" << endl;
    }
    else if (command == "list" && num == 1){
        listGames(player);
        cout << player.getName() << " listed all open games" << endl;
    }
    else if (command == "leave" && num == 1){
        //SendStatus(player.GetSocket(), PLAYER_DISCONNECT);
        client_connected = false;
    }
    else
        valid_cmd = false;

    return valid_cmd;

}
void createJoinGame(Player & player, string game_name){
   
    pthread_mutex_lock(&games_lock);

    auto iter = find_if(game_list.begin(), game_list.end(),
        [game_name] (TTTGame * game) { return game->GetName() == game_name; });

    // Check if the game already exists
    if(iter != game_list.end())
    {
        // Check if the game already has two players
        if((*iter)->GetPlayer2() == -1)
        {
            // If not, join the two players together and notify them
            (*iter)->SetPlayer2(player);
            player.setMode(1);
            SendStatus(player.getSocket(), JOINED);
            SendStatus((*iter)->GetPlayer1().getSocket(), OTHER_JOINED);
        }
        else    // Otherwise, tell the player that game already exists
            SendStatus(player.getSocket(), GAME_EXISTS);
    }
    else
    {
        // Create a new game and add it to the list
        TTTGame * game = new TTTGame(game_name, player);
        game_list.push_back(game);
        //SendStatus(player.GetSocket(), CREATED);
        player.setMode(1);
    }

    pthread_mutex_unlock(&games_lock);
}

// Send a list of all open games to the player
void listGames(Player & player)
{
    int num_open_games = 0;

    // Send a list of all of the games
    SendStatus(player.getSocket(), LIST);
    write(player.getSocket(), "---All Open Games---\n", 21);

    pthread_mutex_lock(&games_lock);
    for(auto iter = game_list.begin(); iter != game_list.end(); ++iter)
    {
        // Only print out the games that don't have a player 2
        if((*iter)->GetPlayer2() == -1) {
            write(player.getSocket(), (*iter)->GetName().c_str(), (*iter)->GetName().length());
            write(player.getSocket(), "\n", 1);
            ++num_open_games;
        }
    }
    pthread_mutex_unlock(&games_lock);

    // If there are no open games, tell the client
    if(num_open_games == 0)
        write(player.getSocket(), "No open games\n", 14);

    // Tell the client we're done sending
    write(player.getSocket(), "\0", 1);
}
void DisconnectPlayer(const Player & player)
{
    // Remove player from any games they were a part of and notify the other player
    pthread_mutex_lock(&games_lock);

    // Inform the other player that this player disconnected
    for(auto iter = game_list.begin(); iter != game_list.end(); ++iter)
    {
        if((*iter)->GetPlayer1() == player)
            continue;
            //SendStatus((*iter)->GetPlayer2().GetSocket(), OTHER_DISCONNECT);
        else if ((*iter)->GetPlayer2() == player)
            continue;
            //SendStatus((*iter)->GetPlayer1().GetSocket(), OTHER_DISCONNECT);
    }

    // Remove any games the player was a part of
    game_list.remove_if([player] (TTTGame * game) { return game->HasPlayer(player); });

    pthread_mutex_unlock(&games_lock);
}

int check(int exp, const char *msg) {
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}
