#pragma once
#include "SFML/Graphics.hpp"
#include "point.h"
#include <vector>

class Player {
public:
    Player(bool wasd, int id);

    point getPos();
    point getMove();

    void checkInput();
    point move();

    void spawn(int x, int y, int dx, int dy);

    void grow(int amount);

    bool dead;
    int id;     // tile id starting at 3
    int score;

    sf::Color color;
private:
    std::vector<point> points;
    point dir, inone, intwo;
    int growth;

    int inputs[4];
};