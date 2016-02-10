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

    void spawnRandom(int id);

    int getW();
    int getH();

    point pos;

private:
    std::vector<int> map;
    int w;
    int h;
};