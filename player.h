#pragma once
#include <string>
using std::string;
class Player{
    private:
        int socket_id;
        string player_name;
        bool ingame;
    public:
        Player(int socket);
        //Player(const Player& p1);
        Player(const Player &copy);
        bool operator==(const Player & other);
        Player * operator=(const Player & other);

        void setName(string name);
        void setMode(bool p);

        int getSocket();
        string getName();
        bool getMode();
};