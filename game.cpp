#include "game.h"

TTTGame::TTTGame(string name, Player player1)
    : m_player1(player1), m_player2(-1), m_name(name)
{ }
Player TTTGame:: GetPlayer1(){
    return m_player1;
}
Player TTTGame:: GetPlayer2(){
    return m_player2;
}
string TTTGame:: GetName(){
    return m_name;
}
void TTTGame:: SetPlayer1(Player m_player1){
    m_player1=m_player1;
} 
void TTTGame:: SetPlayer2(Player m_player2){
    m_player2=m_player2;
}

bool TTTGame::HasPlayer(const Player & player){
    bool hasplayer = false;

    if(m_player1 == player || m_player2 == player)
        hasplayer = true;

    return hasplayer;
}
Player TTTGame::GetOtherPlayer(const Player & player){
    Player other(-1);
    if(m_player1==player)
        other = m_player2;
    else if (m_player2==player)
        other = m_player1;
    return other;
}

