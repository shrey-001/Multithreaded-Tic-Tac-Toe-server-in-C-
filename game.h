#pragma once
#include <string>
#include "player.h"

using std::string;

class TTTGame {
    private:
        Player m_player1;
        Player m_player2;
        string m_name;
    public:
        //Constructor
        TTTGame(string name, Player player1);

        Player GetPlayer1();
        Player GetPlayer2();
        string GetName();

        void SetPlayer1(Player m_player1);
        void SetPlayer2(Player m_player2);
        
        bool HasPlayer(const Player & player);
        Player GetOtherPlayer(const Player & player);

};