// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include "snake.h"
#include "input.h"


Snake::Snake() {
    init();
    start();
}

void Snake::init() {
    // get player ID
    std::string playerID;
    std::cout << "Enter Player ID: ";
    std::getline(std::cin, playerID);

    // get port
    std::string port;
    std::cout << "Enter port number: ";
    std::getline(std::cin, port);
    if (port == "") {
        port = "8000";
        std::cout << "defaulting to port: " << port << std::endl;
    }

    // get ip address
    std::string ip;
    std::cout << "Enter IP address: ";
    std::getline(std::cin, ip);
    if (ip == "") {
        ip = "127.0.0.1";
        std::cout << "defaulting to ip: " << ip << std::endl;
    }

    // try to connect to server
    if (socket.connect(ip, std::atoi(port.c_str())) == sf::Socket::Done) {
        std::cout << "SOCKET CONNECTED!" << std::endl;
        hasConnectedBefore = true;
        sf::Packet p;
        p << (sf::Uint8) 3;
        p << playerID;
        socket.send(p);

    } else {
        std::cout << "SOCKET FAILED TO CONNECT!" << std::endl;
    }
    socket.setBlocking(false);

    // build window
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake Client", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load font
    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FAILED LOADING FONT" << std::endl;
    }
    text.setFont(font);
    text.setCharacterSize(30);

    // set up map
    map = Map((int)(WIDTH / TILE - 8), (int)(HEIGHT / TILE) - 1);
    map.pos = { 0,(int)TILE };
    map.generate();

}

void Snake::gameTick() {
    // extrapolate movement of players
    for (size_t i = 0; i < players.size(); ++i) {
        // figure out dir
        Player& p = players[i];
        point mv = p.getMove();
        if (map.isWalkable(mv.x, mv.y)) {   // valid move
            point oldend = p.move();
            map.setTile(oldend, Map::GROUND);
            map.setTile(p.getPos(), p.id + Map::PLAYER);
        }
    }
    gameFrame++;
}

void Snake::start() {
    bool running = true;
    while (running) {

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

        running = checkServerMessages() && !Input::justPressed(sf::Keyboard::Escape);

        // send ping message
        if (!pingWait) {
            pingWait = true;
            sf::Packet pingPacket;
            pingTime = timeSinceEpochMillis();
            pingPacket << (sf::Uint8)2;
            socket.send(pingPacket);
        }

        // make sure index is valid and window is focused	
        if (playerIndex >= 0 && playerIndex < static_cast<int>(players.size()) && window->hasFocus()) {
            Player& myp = players[playerIndex];

            if (myp.checkInput()) {
                sf::Packet packet;
                packet << (sf::Uint8) 0;
                packet << gameFrame;
                packet << myp.inone;
                packet << myp.intwo;
                socket.send(packet);
            }

        }

        if (gameRunning) {
            unsigned curTick = timeSinceEpochMillis() / msPerTick;
            // if enough time has passed then do a game tick
            if (curTick > latestTick) {
                latestTick = curTick;
                gameTick(); // purely for extrapolation
            }
        }

        // renders the board and limits framerate
        render();
    }

    socket.disconnect();

    // cleanup
    delete window;
}

bool Snake::checkServerMessages() {
    sf::Packet packet;
    while (true) {  // loop until you run out of packets in buffer
        sf::Socket::Status status = socket.receive(packet);
        if (status == sf::Socket::NotReady || status == sf::Socket::Error) {
            return true;
        } else if (status == sf::Socket::Disconnected) {
            return !hasConnectedBefore;
        }
        processPacket(packet);
    }
    return true;
}

// process packet from server
void Snake::processPacket(sf::Packet& packet) {
    sf::Uint8 b;    // make sure to unpack it as type it was packed as
    packet >> b;
    int type = b;

    if (type == 0) {    // game state update
        // packet layout defined in server snake.cpp
        // builds local player vector for getting input and score reference
        // will be used later to extrapolate player movement during lag

        if (!gameRunning) {
            gameRunning = true;
            gameStartTime = timeSinceEpochMillis();
        }
        // check to see if server state is newer than current server state
        unsigned int updateTime;
        packet >> updateTime;
        // dont update if its an older update
        if (updateTime <= lastServerUpdate) {
            return;
        }
        lastServerUpdate = updateTime;

        packet >> b;
        int numPlayers = b;
        std::vector<Player> serverPlayers;
        for (int i = 0; i < numPlayers; ++i) {
            packet >> b;
            int playerid = b;
            Player player(playerid);
            packet >> player.playerName;
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
            }
            serverPlayers.push_back(player);
        }
        packet >> foodPos;
        // update local state to server state
        map.generate();
        map.setTile(foodPos, Map::FOOD);
        players = serverPlayers;
        for (size_t i = 0; i < players.size(); ++i) {
            auto& ppoints = players[i].getPoints();
            for (size_t j = 0; j < ppoints.size(); ++j) {
                map.setTile(ppoints[j], players[i].id + Map::PLAYER);
            }
        }

        // set playerIndex
        packet >> b;
        playerIndex = b;

    } else if (type == 1) { // update title text
        packet >> titleText;
    } else if (type == 2) {
        pingVector.push_back(timeSinceEpochMillis() - pingTime);
        pingWait = false;

        if (pingVector.size() > 100) {   // lowered this for when laggy
            pingVector.erase(pingVector.begin());
        }
    } else {
        std::cout << "Unknown packet type received from server" << std::endl;
    }
}

unsigned Snake::timeSinceEpochMillis() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count());
}


void Snake::render() {
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    map.generateVertices(verts, TILE);

    window->draw(verts);

    // render player score text
    // currently hardcoded for two players (will need to be changed eventually)
    for (size_t i = 0; i < players.size(); ++i) {
        std::ostringstream oss;

        Player& p = players[i];
        oss << p.playerName << " : " << p.score << " ";
        text.setString(oss.str());
        text.setColor(Map::getColorFromID(p.id + Map::PLAYER));

        text.setPosition(sf::Vector2f(800.0f, i * 50.0f + 50.0f));

        // add black box to this clients player score
        float rectW = 200.0f;
        if (i == playerIndex) {
            sf::RectangleShape rs;
            rs.setPosition(sf::Vector2f(800.0f, i * 50.0f + 50.0f));
            rs.setSize(sf::Vector2f(rectW, 50.0f));
            rs.setFillColor(sf::Color::Black);
            window->draw(rs);

        }

        // draw text after box
        window->draw(text);
    }

    // Display ping in top left corner of the screen
    long long ping_avg = 0;
    for (size_t i = 0; i < pingVector.size(); i++) {
        ping_avg += pingVector[i];
    }
    if (pingVector.size() > 0) {
        ping_avg /= pingVector.size();
    }
    text.setColor(sf::Color(255, 0, 0));
    text.setString("Ping: " + std::to_string(ping_avg) + " ms");
    text.setPosition(sf::Vector2f(0.0f, 0.0f));
    window->draw(text);


    // set title message text
    text.setColor(sf::Color(200, 255, 255));
    text.setString(titleText);
    sf::FloatRect fr = text.getLocalBounds();
    text.setPosition(sf::Vector2f(WIDTH / 2 - fr.width / 2.0f, 0.0f));
    window->draw(text);

    // swap buffers
    window->display();
}

int main() {
    Snake game;
    return 0;
}