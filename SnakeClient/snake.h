// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#pragma once
#include "map.h"
#include "player.h"
#include <SFML/Network.hpp>

class Snake {
public:
    Snake();

    const int WIDTH = 800;
    const int HEIGHT = 600;
    const float TILE = 25.0f;

    static const int GROUND = 0;
    static const int WALL = 1;
    static const int FOOD = 2;
    static const int PLAYER = 3;
private:
    sf::RenderWindow* window;

    sf::TcpSocket socket;

    // which player you are in list of players
    int playerIndex;

    // will be used as a holder for scores
    // and when getting input say players[playerIndex].getInput()
    std::vector<Player> players;

    bool gameRunning = false;

    Map map;
    sf::Font font;
    sf::Text text;


    void init();
    void start();

    char in[256];
    std::size_t received_len;

    void checkServerMessage();
    void clearMessageBuffer();
    
    template <typename T>
    void sendData(T data);

    void render();
    void generateVertices(sf::VertexArray& verts);

};
