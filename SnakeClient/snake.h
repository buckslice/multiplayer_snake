#pragma once
#include "map.h"
#include "player.h"

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
    Map map;
    sf::Font font;
    sf::Text text;

    std::vector<Player> players;

    void init();
    void start();
    void render();

    int winner;
    int getWinner();
    void spawnPlayers();

    float gameTime;

    void generateVertices(sf::VertexArray& verts);

};
