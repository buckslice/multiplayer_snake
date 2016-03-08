// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include <vector>
#include "point.h"
#include <SFML/Graphics.hpp>

class Map {
public:
    Map();
    Map(int w, int h);

    int getTile(int x, int y);

    void setTile(int x, int y, int id);
    void setTile(point p, int id);

    bool isWalkable(int x, int y);

    // generates an empty board
    void generate();

    void generateVertices(sf::VertexArray& verts, float tileSize);

    // returns where it spawned it
    point spawnRandom(int id);

    int getW();
    int getH();

    point pos; // offset of map origin

    // tile IDs
    static const int GROUND = 0;
    static const int WALL = 1;
    static const int FOOD = 2;
    static const int PLAYER = 3;

    static sf::Color getColorFromID(int id);

private:
    std::vector<int> map;
    int w;
    int h;
};