// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>

#include "snake.h"
#include "input.h"

Snake::Snake() {
    init();
    start();
}

void Snake::init() {
    std::string port;
    std::cout << "Enter port number: ";
    std::getline(std::cin, port);
    if (port == "") {
        port = "8000";
        std::cout << "defaulting to port: " << port << std::endl;
    }

    std::string ip;
    std::cout << "Enter IP address: ";
    std::getline(std::cin, ip);
    if (ip == "") {
        ip = "127.0.0.1";
        std::cout << "defaulting to ip: " << ip << std::endl;
    }

    if (socket.connect(ip, std::atoi(port.c_str())) != sf::Socket::Done) {
        std::cout << "SOCKET FAILED TO CONNECT!" << std::endl;
    } else {
        std::cout << "SOCKET CONNECTED!" << std::endl;
    }

    socket.setBlocking(false);
    sendData("Hello from the client!");

    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake!", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FONT COULDN'T LOAD" << std::endl;
    }
}

void Snake::start() {
    map = Map((int)(WIDTH / TILE), (int)(HEIGHT / TILE) - 1);
    map.pos = { 0,(int)TILE };
    map.generate();

    text.setFont(font);
    text.setCharacterSize(30);

    // set up players
    int ids = PLAYER;
    players.push_back(Player(true, ids++));
    players.push_back(Player(false, ids++));

    // set up player colors
    players[0].color = sf::Color(255, 127, 51);
    players[1].color = sf::Color(255, 255, 51);

    spawnPlayers();
    map.spawnRandom(FOOD);

    gameTime = -1.0f;
    render();
    winner = 0;
    sf::Clock frameTime;
    bool running = true;
    while (running) {
        // increment gameTime
        float delta = frameTime.restart().asSeconds();
        gameTime += delta;

        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            default:
                break;
            }
        }

        // update input arrays
        Input::update();

        if (Input::justPressed(sf::Keyboard::Key::Escape)) {
            running = false;
        }

        // get input from players
        for (size_t i = 0; i < players.size(); ++i) {
            players[i].checkInput();
        }

        float gameTick = 0.1f;  // snake moves 10 times a second
        if (gameTime >= gameTick) {
            std::string txt;
            for (size_t i = 0; i < players.size(); ++i) {
                // figure out dir
                Player& p = players[i];
                point mv = p.getMove();
                if (map.isWalkable(mv.x, mv.y)) {   // valid move
                    point oldend = p.move();
                    map.setTile(oldend, 0);
                    map.setTile(p.getPos(), p.id);
                } else if (map.getTile(mv.x, mv.y) == FOOD) {   // ran into food
                    point oldend = p.move();
                    map.setTile(oldend, 0);
                    map.setTile(p.getPos(), p.id);
                    p.grow(3);
                    //p.score++;

                    sendData(i + 1);

                    map.spawnRandom(FOOD);
                } else {    // hit wall or part of a snake so this player dies!
                    p.dead = true;
                }
            }

            winner = getWinner();
            // if game ended then restart
            if (winner != 0) {
                gameTime = -2.0f;
            } else {    // else just zero it cuz games still going
                gameTime = 0.0f;
            }

            render();
        }

        // reset game after 1 second
        if (winner != 0 && gameTime > -1.0f) {
            winner = 0;
            sendData('0');
            map.generate();
            spawnPlayers();
            map.spawnRandom(FOOD);
            render();
        }

    }

    socket.disconnect();

    // cleanup
    delete window;
}

template <typename T>
void Snake::sendData(T data) {
    std::ostringstream oss;
    oss << data;
    std::string s = oss.str();
    size_t len;
    socket.send(s.c_str(), s.length(), len);
}

// return 0 means game still going
// return -1 means draw
// return > 0 means player n won
int Snake::getWinner() {
    bool oneAlive = false;
    int winner = 0;
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[i].dead) {
            if (oneAlive) { // if another player has been found alive then return -1 cuz game isnt over
                return 0;
            }
            winner = i + 1;
            oneAlive = true;
        }
    }
    if (!oneAlive) {  // this means it was a draw
        return -1;
    }

    return winner;
}

void Snake::spawnPlayers() {
    for (size_t i = 0; i < players.size(); ++i) {
        Player& p = players[i];
        switch (p.id - PLAYER) {
        case 0:
            p.spawn(4, 4, 0, 1);
            break;
        case 1:
            p.spawn(map.getW() - 5, map.getH() - 5, 0, -1);
            break;
        default:
            std::cout << "spawning player at default spawn" << std::endl;
            p.spawn(map.getW() / 2, map.getH() / 2, 1, 0);
            break;
        }
        map.setTile(p.getPos(), p.id);
    }
}

void Snake::clearMessage() {
    int len = sizeof(in) / sizeof(char);
    for (int i = 0; i < len; ++i) {
        in[i] = '\0';
    }
}

void Snake::getMessage() {
    // check for scores from server
    if (socket.receive(in, sizeof(in), received) == sf::Socket::NotReady) {
        //std::cout << "discarding partial message" << std::endl;
        clearMessage();
    }

    std::string msg = std::string(in);
    if (msg == "") {
        return;
    }

    for (size_t i = 0; i < msg.size(); ++i) {
        players[i].score = msg.at(i) - '0';
    }

    clearMessage();

}

void Snake::render() {
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    generateVertices(verts);

    window->draw(verts);

    getMessage();

    for (size_t i = 0; i < players.size(); ++i) {
        std::ostringstream oss;

        Player& p = players[i];
        oss << "P " << p.id - PLAYER + 1 << " : " << p.score << " ";
        text.setString(oss.str());
        text.setColor(p.color);
        if (i == 0) {
            text.setPosition(sf::Vector2f(0.0f, 0.0f));
        } else {
            sf::FloatRect fr = text.getLocalBounds();
            text.setPosition(sf::Vector2f(WIDTH - fr.width, 0.0f));
        }
        window->draw(text);
    }

    text.setColor(sf::Color(200, 255, 255));
    std::ostringstream oss;
    if (gameTime < -1.0f) {
        if (winner < 0) {
            oss << "DRAW!";
        } else {
            oss << "Player " << winner << " wins!";
        }
    } else if (gameTime < 0.0f) {
        oss << "GET READY";
    } else {
        oss << "SNAKE!";
    }
    text.setString(oss.str());
    sf::FloatRect fr = text.getLocalBounds();
    text.setPosition(sf::Vector2f(WIDTH / 2 - fr.width / 2.0f, 0.0f));
    window->draw(text);

    window->display();
}

void Snake::generateVertices(sf::VertexArray& verts) {
    for (int y = 0; y < map.getH(); ++y) {
        for (int x = 0; x < map.getW(); ++x) {
            int id = map.getTile(x, y);

            float x0 = x * TILE + map.pos.x;
            float y0 = y * TILE + map.pos.y;
            float x1 = (x + 1) * TILE + map.pos.x;
            float y1 = (y + 1) * TILE + map.pos.y;

            sf::Color color;
            switch (id) {
            case GROUND:
                color = sf::Color(51, 77, 77);
                break;
            case WALL:
                color = sf::Color(12, 26, 51);
                break;
            case FOOD:
                color = sf::Color(0, 255, 0);
                break;
            default:
                Player& p = players[id - PLAYER];
                if (gameTime < 0.0 && p.dead) {
                    color = sf::Color(255, 0, 0);
                } else {
                    color = p.color;
                }
                break;
            }

            verts.append(sf::Vertex(sf::Vector2f(x0, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y0), color));
            verts.append(sf::Vertex(sf::Vector2f(x1, y1), color));
            verts.append(sf::Vertex(sf::Vector2f(x0, y1), color));
        }
    }
}

int main() {

    Snake game;
    return 0;
}