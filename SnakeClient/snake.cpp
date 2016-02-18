// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Patches

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

    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake Client", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FAILED LOADING FONT" << std::endl;
    }

    map = Map((int)(WIDTH / TILE), (int)(HEIGHT / TILE) - 1);
    map.pos = { 0,(int)TILE };
    map.generate();

    text.setFont(font);
    text.setCharacterSize(30);
}

void Snake::start() {

    sf::Clock frameTime;
    bool running = true;
    while (running) {
        float delta = frameTime.restart().asSeconds();

        // process window events
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

        if (Input::justPressed(sf::Keyboard::Escape)) {
            running = false;
        }

        checkServerMessages();

        if (playerIndex < 0 || playerIndex >= players.size()) {
            // game not started yet or index set up wrong
        } else if (window->hasFocus()) {

            Player& myp = players[playerIndex];
            myp.checkInput();

            sf::Packet packet;
            packet << (sf::Uint8) 0;
            packet << myp.inone;
            packet << myp.intwo;
            socket.send(packet);

        }

        // renders the board and limits framerate
        render();
    }

    socket.disconnect();

    // cleanup
    delete window;
}

void Snake::checkServerMessages() {
    sf::Packet packet;
    while (true) {
        sf::Socket::Status status = socket.receive(packet);
        if (status != sf::Socket::Done) {
            return;
        }
        processPacket(packet);
    }
}

void Snake::processPacket(sf::Packet& packet) {
    sf::Uint8 b;
    packet >> b;
    int type = b;

    if (type == 0) {    // game state update
        packet >> b;
        int numPlayers = b;

        map.generate();
        players.clear();
        for (int i = 0; i < numPlayers; ++i) {
            packet >> b;
            int playerid = b;
            Player player(playerid);
            packet >> b;
            int playerScore = b;
            player.score = playerScore;
            packet >> player.dir;
            packet >> player.inone;
            packet >> player.intwo;
            packet >> b;
            int numPoints = b;
            auto& points = player.getPoints();
            for (int j = 0; j < numPoints; ++j) {
                point p;
                packet >> p;
                points.push_back(p);
                map.setTile(p, player.id + PLAYER);
            }
            players.push_back(player);
        }

        point p;
        packet >> p;
        map.setTile(p, FOOD);

        packet >> b;
        playerIndex = b;


    } else if (type == 1) { // update title text
        packet >> titleText;
    } else {
        std::cout << "Unknown packet type received from server" << std::endl;
    }
}

void Snake::render() {
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    generateVertices(verts);

    window->draw(verts);

    for (size_t i = 0; i < players.size(); ++i) {
        std::ostringstream oss;

        Player& p = players[i];
        oss << "P " << (p.id + 1) << " : " << p.score << " ";
        text.setString(oss.str());
        sf::Color c = Player::getColorFromID(p.id);
        text.setColor(c);
        if (i == 0) {
            text.setPosition(sf::Vector2f(0.0f, 0.0f));
        } else {
            sf::FloatRect fr = text.getLocalBounds();
            text.setPosition(sf::Vector2f(WIDTH - fr.width, 0.0f));
        }

        float rectW = 150.0f;
        if (i == playerIndex) {
            sf::RectangleShape rs;
            if (i == 0) {
                rs.setPosition(sf::Vector2f(0.0f, 0.0f));
            } else {
                rs.setPosition(sf::Vector2f(WIDTH - rectW, 0.0f));
            }
            rs.setSize(sf::Vector2f(rectW, 50.0f));
            rs.setFillColor(sf::Color::Black);
            window->draw(rs);
        }

        window->draw(text);
    }

    text.setColor(sf::Color(200, 255, 255));
    text.setString(titleText);
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
                color = Player::getColorFromID(id - PLAYER);
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