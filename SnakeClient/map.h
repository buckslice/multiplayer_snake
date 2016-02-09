#pragma once
#include <vector>
#include <GL/glew.h>
#include "point.h"

class Map {
public:
    Map();

    int getTile(int x, int y);

    void setTile(int x, int y, int id);
    void setTile(point p, int id);

    bool isWalkable(int x, int y);

    void generate(int w, int h);

    void spawnRandom(int id);

    int getW();
    int getH();

    //static const GLuint GROUND = 0;
    //static const GLuint WALL = 1;
    //static const GLuint FOOD = 2;

private:
    std::vector<int> map;
    int w;
    int h;
};