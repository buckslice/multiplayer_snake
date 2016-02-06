#include "map.h"
#include <random>

std::random_device rd;
std::mt19937 rng(rd());

Map::Map() {
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
    return getTile(x, y) == 0;
}

void Map::spawnRandom(int id) {
    std::vector<point> open;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (map[x + y*w] == 0) {
                open.push_back({ x,y });
            }
        }
    }

    std::uniform_int_distribution<int> uni(0, open.size() - 1);
    setTile(open[uni(rng)], id);
}

void Map::generate(int w, int h) {
    this->w = w;
    this->h = h;
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