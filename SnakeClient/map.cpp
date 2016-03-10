// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#include "map.h"
#include <random>

std::random_device rd;
std::mt19937 rng(rd());

Map::Map() {
    pos = { 0,0 };
}

Map::Map(int w, int h) :w{ w }, h{ h } {
    pos = { 0,0 };
}

int Map::getW() {
    return w;
}
int Map::getH() {
    return h;
}

int Map::getTile(int x, int y) {
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return -1;
    } else {
        return map[x + y * w];
    }
}

void Map::setTile(int x, int y, int id) {
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return;
    }
    map[x + y * w] = id;
}

void Map::setTile(point p, int id) {
    setTile(p.x, p.y, id);
}

bool Map::isWalkable(int x, int y) {
    int tile = getTile(x, y);
    return tile == GROUND || tile == FOOD;
}

void Map::generate() {
    map.clear();
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (y == 0 || y == h - 1 || x == 0 || x == w - 1) {
                map.push_back(1);
            } else {
                map.push_back(0);
            }
        }
    }
}

// generates a vertex array from map data
void Map::generateVertices(sf::VertexArray& verts, float tileSize) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int id = getTile(x, y);

            float x0 = x * tileSize + pos.x;
            float y0 = y * tileSize + pos.y;
            float x1 = (x + 1) * tileSize + pos.x;
            float y1 = (y + 1) * tileSize + pos.y;

            sf::Color color = getColorFromID(id);

            verts.append(sf::Vertex(sf::Vector2f(x0, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y1), color));
            verts.append(sf::Vertex(sf::Vector2f(x0, y1), color));
        }
    }
}

sf::Color Map::getColorFromID(int id) {
    switch (id) {
    case GROUND:
        return sf::Color(51, 77, 77);
    case WALL:
        return sf::Color(12, 26, 51);
    case FOOD:
        return sf::Color(0, 255, 0);
    default:
        switch (id - PLAYER) {
        case 0:
            return sf::Color(255, 127, 51);
        case 1:
            return sf::Color(255, 255, 51);
        default:
            return sf::Color(255, 0, 255);
        }
    }
}