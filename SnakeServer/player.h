// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#pragma once
#include "SFML/Graphics.hpp"
#include "point.h"
#include <vector>

class Player {
public:
    Player(int id);

    point getPos();
    std::vector<point>& getPoints();
    point getMove();    // calculates dir based on inputs

    point move();
    point dir, inone, intwo;

    void spawn(int x, int y, int dx, int dy);

    void grow(int amount);

    bool dead;
    int id;
    int score;

    static sf::Color getColorFromID(int id);
private:
    std::vector<point> points;
    int growth;
};