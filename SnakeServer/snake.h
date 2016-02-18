// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Patches

#pragma once
#include <SFML/Network.hpp>
#include "map.h"
#include "player.h"

class Snake {
public:
    Snake();
    ~Snake();

    const int WIDTH = 800;
    const int HEIGHT = 600;
    const float TILE = 25.0f;

    static const int GROUND = 0;
    static const int WALL = 1;
    static const int FOOD = 2;
    static const int PLAYER = 3;
private:
    sf::RenderWindow* window;

    sf::TcpListener listener;

    // client position in list are ordered by longest time connected
    std::vector<sf::TcpSocket*> clients;

    // client[0] is always player[0] and so on, but ids may mismatch depending on disconnects
    // client 1 might be player 2 for example
    std::vector<Player> players;

    Map map;
    sf::Font font;
    sf::Text text;
    point foodPos;


    void init();
    void start();

    void checkNewConnections();
    void checkClientMessages();
    void processPacket(sf::Packet& packet, int index);

    int winner;
    bool gameRunning = false;
    float gameTime = -100.0f;
    const float tickTime = 0.1f;    // should slow down when testing

    void gameTick();
    void broadcastGameState();
    void broadcastPacket(sf::Packet& packet);    // send packet to all clients
    int getWinner();
    void startGame(float delay);

    std::string getTitle();
    void render();
    void generateVertices(sf::VertexArray& verts);


};
