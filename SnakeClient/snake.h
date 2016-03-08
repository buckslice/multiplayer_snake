// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include "map.h"
#include "player.h"
#include <SFML/Network.hpp>

class Snake {
public:
    Snake();

    const int WIDTH = 1000;
    const int HEIGHT = 600;
    const float TILE = 10.0f;

private:
    sf::RenderWindow* window;

    sf::TcpSocket socket;

    // which player you are in list of players
    int playerIndex = -1;

    // will be used as a holder for scores
    // and when getting input say players[playerIndex].getInput()
    std::vector<Player> players;
    //unsigned int clientStateID = -1;			// Counter for current gamestate
    //unsigned int serverStateID;
    unsigned serverFrame;

    bool gameRunning = false;
    unsigned gameFrame = 0;
    unsigned latestTick;
    unsigned gameStartTime;
    const unsigned msPerTick = 500;

    std::string titleText;
    std::string pingText;
	
    // for determining ping
    bool pingWait = false;
    unsigned pingTime;
    std::vector<unsigned> pingVector;

    bool shouldSendInput = false;

    Map map;
    point foodPos;

    sf::Font font;
    sf::Text text;

    void init();
    void gameTick();    // progresses state of game by one game tick
    void start();

    // returns false when server is disconnected
    bool checkServerMessages();
    void processPacket(sf::Packet& packet);

    void render();

    unsigned timeSinceEpochMillis();

    bool hasConnectedBefore = false;

};
