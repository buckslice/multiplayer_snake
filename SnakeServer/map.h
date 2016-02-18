// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#pragma once
#include <vector>
#include "point.h"

class Map {
public:
    Map();
    Map(int w, int h);

    int getTile(int x, int y);

    void setTile(int x, int y, int id);
    void setTile(point p, int id);

    bool isWalkable(int x, int y);

    void generate();

    // returns where it spawned it
    point spawnRandom(int id);

    int getW();
    int getH();

    point pos;

private:
    std::vector<int> map;
    int w;
    int h;
};