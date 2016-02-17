// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

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
    std::vector<sf::TcpSocket*> clients;

    std::vector<Player> players;

    Map map;
    sf::Font font;
    sf::Text text;


    void init();
    void start();

    void checkNewConnections();
    void checkClientMessages();

    int winner;
    bool gameRunning = false;
    float gameTime = -100.0f;
    const float tickTime = 0.1f;

    void gameTick();
    void broadcastGameState();
    int getWinner();
    void startGame(float delay);

    void render();
    void generateVertices(sf::VertexArray& verts);


    char in[128];
    std::size_t received_len;
    void clearMessageBuffer();

    //void getMessage();
    //void clearMessage();

    //template <typename T>
    //void sendData(T data);

};
