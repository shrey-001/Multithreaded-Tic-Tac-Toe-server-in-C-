#include "player.h"

Player::Player(int socket)
    : socket_id(socket), player_name("No Name"), ingame(0)
{ }

// Copy constructor
Player::Player(const Player &copy)
    : socket_id(-1), player_name("No Name"), ingame(0)
{
    *this = copy;
}
bool Player::operator==(const Player & other){
    return player_name == other.player_name;
}
Player * Player::operator=(const Player & other){
    if(&other != this){
        socket_id = other.socket_id;
        player_name = other.player_name;
        ingame = other.ingame;
    }
    return this;
}

void Player::setName(string name){
    player_name=name;
}
void Player::setMode(bool p){
    ingame=p;
}

int Player::getSocket(){
    return socket_id;
}
string Player:: getName(){
    return player_name;
}
bool Player::getMode(){
    return ingame;
}