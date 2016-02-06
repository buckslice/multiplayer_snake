#pragma once
#include <GLFW/glfw3.h>
#include "point.h"
#include <vector>

class Player {
public:
    point getPos();
    point getMove();

    void checkInput();
    point move();

    void spawn(int x, int y, int dx, int dy);

    void grow(int amount);

    bool dead;

private:
    std::vector<point> points;
    point dir, inone, intwo;
    int growth;
};