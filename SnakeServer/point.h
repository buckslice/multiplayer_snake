// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis

#include "SFML/Network.hpp"
#pragma once

struct point {  // should prob just switch to sf::Vector2i now that we are using SFML
    int x, y;
    struct point& operator+=(const point& p) { x += p.x; y += p.y; return *this; }
    friend point operator+(const point& p1, const point& p2) {
        return{ p1.x + p2.x, p1.y + p2.y };
    }
    friend sf::Packet& operator << (sf::Packet& packet, const point& p) {
        return packet << (sf::Int8)p.x << (sf::Int8)p.y;
    }
    friend sf::Packet& operator >> (sf::Packet& packet, point& p) {
        sf::Int8 b;
        packet >> b;
        p.x = b;
        packet >> b;
        p.y = b;
        return packet;
    }
    friend std::ostream& operator << (std::ostream& os, const point& p) {
        os << p.x << " " << p.y;
        return os;
    }
};

