// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis

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

    // generates an empty board
    void generate();

    int getW();
    int getH();

    point pos;  // offset of map origin

private:
    std::vector<int> map;
    int w;
    int h;
};