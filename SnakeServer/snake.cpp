// ICS 167 Multiplayer Snake Project by:
// John Collins     75665849    jfcollin@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// Matt Ruiz        28465978    mpruiz@uci.edu
// Gary Machlis		13792598	gmachlis@uci.edu

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>

#include "snake.h"

Snake::Snake() {
    init();
    start();
}

Snake::~Snake() {
    listener.close();

    // delete client sockets pointers
    for (size_t i = 0, len = clients.size(); i < len; ++i) {
        clients[i]->disconnect();
        delete clients[i];
    }
    clients.erase(clients.begin(), clients.end());

    delete window;

}


void Snake::init() {
    // get port number
    std::string port;
    std::cout << "Enter port number: ";
    std::getline(std::cin, port);
    if (port == "") {
        port = "8000";
        std::cout << "defaulting to port: " << port << std::endl;
    } else {
        std::cout << port << std::endl;
    }

    // bind the listener to a port
    listener.setBlocking(false);
    if (listener.listen(std::atoi(port.c_str())) != sf::Socket::Done) {
        std::cout << "CANT LISTEN AT THIS PORT" << std::endl;
    }

    std::cout << "SERVER STARTED..." << std::endl;

    // build window to use for game loop and for rendering game state
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 2;
    //settings.majorVersion = 3.0;
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "Snake Server", sf::Style::Default, settings);
    window->setFramerateLimit(60);

    // load font
    if (!font.loadFromFile("OCRAEXT.TTF")) {
        std::cout << "FAILED LOADING FONT" << std::endl;
    }
    text.setFont(font);
    text.setCharacterSize(30);

    // set up map
    map = Map((int)(WIDTH / TILE - 20), (int)(HEIGHT / TILE) - 3);
    map.pos = { 0,(int)TILE*3 };
    map.generate();

    // init latency simulation stuff
    std::random_device rd;
    rng.seed(rd());

    clientDelays.push_back(point{ 0, 100 });
    clientDelays.push_back(point{ 500, 1000 });
}

void Snake::checkNewConnections() {
    // always keep one open connection at end of client list that listens for new connections
    if (clients.size() == players.size()) {
        sf::TcpSocket* client = new sf::TcpSocket();
        client->setBlocking(false);
        clients.push_back(client);
        
    }

    // if successfully get new connection then add player to list
    if (listener.accept(*clients[players.size()]) == sf::Socket::Done) {

        // find a free id for new player
        int freeID = 0;
        bool foundFree = false;
        while (!foundFree) {
            foundFree = true;
            for (size_t i = 0; i < players.size(); ++i) {
                if (players[i].id == freeID) {
                    freeID++;
                    foundFree = false;
                }
            }
        }

        players.push_back(Player(freeID));    // player ids are 0 indexed
        std::cout << "New client connected, set as player " << players.size() << std::endl;
        inputBuffer.resize(players.size());

        // start game if enough players
        if (players.size() >= 2) {
            resetGame();
            gameStartTime = timeSinceEpochMillis() + 2000;
        }
    }
}

void Snake::checkClientMessages() {
    // iterate over each connected client and check for received messages
    // skip last element of clients since that is an open socket waiting for a new connection
    for (size_t i = 0, len = clients.size() - 1; i < len; ++i) {
        sf::Packet packet;

        // loop until all packets are processed
        while (true) {
            // get packet from client socket
            sf::Socket::Status status = clients[i]->receive(packet);

            // if no data from the client this frame or some sort of error then continue (by breaking infinite while)
            if (status == sf::Socket::NotReady || status == sf::Socket::Error) {
                break;
            } else if (status == sf::Socket::Disconnected) {    // if client disconnected
                std::cout << "Client " << (i + 1) << " disconnected" << std::endl;

                // erase client info from vectors
                delete(clients[i]);
                clients.erase(clients.begin() + i);
                players.erase(players.begin() + i);
                inputBuffer.resize(players.size());

                if (players.size() < 2) {
                    // resets game while waiting
                    gameStartTime = -1;
                    winner = 0;
                    map.generate();
                }
                break;
            }
            // reaching this point means data was successfully received

            // process packet
            if (addReceiveLatency) {
                delayReceivedList.push_back(DelayedPacket(packet, i, getDelay(i)));
            } else {
                processPacket(packet, i);
            }
        }
    }
}

void Snake::processPacket(sf::Packet& packet, int index) {
    sf::Uint8 b;
    packet >> b;
    if (b == 0) {           // input update from client
        unsigned clientFrame;   // what frame client made input
        packet >> clientFrame;

        if (clientFrame < earliestFrame) {
            earliestFrame = clientFrame;
        }
        receivedInputs++;

        // extract input and put onto input list
        packet >> inputBuffer[index][clientFrame];

    } else if (b == 1) {    // any sort of string message from client
        std::string s;
        packet >> s;

        std::cout << s << std::endl;
    } else if (b == 2) {    // ping check
        sf::Packet pingback;
        pingback << (sf::Uint8) 2;
        sendPacket(pingback, index);
    } else if (b == 3) {    // client sending player name
        std::string s;
        packet >> s;
        s = s.substr(0, 5);
        players[index].playerName = s;
    } else {
        std::cout << "Unknown packet type received from client" << std::endl;
    }
}

void Snake::broadcastPacket(sf::Packet& packet) {
    for (size_t i = 0; i < clients.size() - 1; ++i) {
        sendPacket(packet, i);
    }
}

void Snake::sendPacket(sf::Packet& packet, int clientIndex) {
    if (addSendLatency) {
        delaySendList.push_back(DelayedPacket(packet, clientIndex, getDelay(clientIndex)));
    } else {
        clients[clientIndex]->send(packet);
    }
}

unsigned Snake::getDelay(int index) {
    if (index < 0 || index >= static_cast<int>(clientDelays.size())) {
        std::uniform_int_distribution<unsigned> uni(100, 300);
        return timeSinceEpochMillis() + uni(rng);
    } else {
        point p = clientDelays[index];
        std::uniform_int_distribution<unsigned> uni(p.x, p.y);
        return timeSinceEpochMillis() + uni(rng);
    }
}

void Snake::broadcastGameState() {
    // PACKET LAYOUT
    // type
    // game state id
    // number of players
    // for each player
        // player name
        // player id
        // player score
        // player dir
        // length of snake
        // points in snake
    // food pos
    // id of client in player list

    sf::Packet packet;

    packet << (sf::Uint8) 0;
    packet << gameFrame;
    packet << (sf::Uint8) players.size();
    for (size_t i = 0; i < players.size(); ++i) {
        Player& player = players[i];
        packet << (sf::Uint8)player.id;
        packet << player.playerName;
        packet << (sf::Uint8)player.score;
        packet << player.dir;

        auto& points = player.getPoints();
        packet << (sf::Uint8) points.size();

        for (size_t j = 0; j < points.size(); ++j) {
            packet << points[j];
        }

    }
    packet << foodPos;

    // send game state to each client
    for (size_t i = 0; i < clients.size() - 1; ++i) {
        sf::Packet clientPacket = packet;
        clientPacket << (sf::Uint8) i;
        sendPacket(clientPacket, i);
    }
}

void Snake::checkAndSendDelayed() {
    if (!addSendLatency || delaySendList.empty()) {
        return;
    }

    unsigned curTime = timeSinceEpochMillis();
    size_t i = 0;
    int numClients = clients.size();
    while (i < delaySendList.size()) {
        DelayedPacket& dp = delaySendList[i];

        if (dp.readyTime < curTime) {
            if (dp.clientIndex < numClients) {
                // send packet if it waited enough
                clients[dp.clientIndex]->send(dp.packet);
            }

            // remove packet from list
            std::swap(delaySendList[i], delaySendList.back());
            delaySendList.pop_back();
        } else {
            i++;
        }
    }
}

void Snake::checkAndReceiveDelayed() {
    if (!addReceiveLatency || delayReceivedList.empty()) {
        return;
    }

    unsigned curTime = timeSinceEpochMillis();
    size_t i = 0;
    while (i < delayReceivedList.size()) {
        DelayedPacket& dp = delayReceivedList[i];

        if (dp.readyTime < curTime) {
            // process packet if it waited enough
            processPacket(dp.packet, dp.clientIndex);

            // remove packet from list
            std::swap(delayReceivedList[i], delayReceivedList.back());
            delayReceivedList.pop_back();
        } else {
            i++;
        }
    }
}

unsigned Snake::timeSinceEpochMillis() {
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count());
}


// progress state of game by one move
void Snake::gameTick() {
    for (size_t i = 0; i < players.size(); ++i) {
        // figure out dir
        Player& p = players[i];
        point mv = p.getPos() + p.dir;
        int tile = map.getTile(mv.x, mv.y);
        if (map.isWalkable(mv.x, mv.y)) {   // valid move
            point oldend = p.move();
            map.setTile(oldend, Map::GROUND);
            map.setTile(p.getPos(), p.id + Map::PLAYER);

            if (tile == Map::FOOD) {   // ran into food
                p.grow(3);
                p.score++;

                foodPos = map.spawnRandom(Map::FOOD);
            }
        } else {    // hit wall or part of a snake so this player dies!
            //std::cout << p.dir << " " << p.inone << " " << p.intwo << std::endl;
            p.dead = true;
        }
    }
}

void Snake::resimulateGameToPresentState() {
    // get number of previous states
    int numPrevious = pastStates.size();
    // if no previous states then return because no data to retest from
    if (numPrevious <= 0) {
        return;
    }
    // get index of previous frame to start from
    int index = numPrevious - 1 - (gameFrame - earliestFrame);
    if (index < 0) {
        index = 0;
    } else if (index >= numPrevious) {
        index = numPrevious - 1;
    }

    // reset game back to a previous state
    unsigned simFrame = pastStates[index].gameFrame;
    players = pastStates[index].playerVector;
    map.generate();
    map.setTile(foodPos, Map::FOOD);
    for (size_t i = 0; i < players.size(); ++i) {
        auto& ppoints = players[i].getPoints();
        for (size_t j = 0; j < ppoints.size(); ++j) {
            map.setTile(ppoints[j], players[i].id + Map::PLAYER);
        }
    }

    //std::cout << earliestFrame << " " << simFrame << " " << gameFrame << std::endl;
    
    // resimulate game up to present point but using the inputs
    // of each player during the frame they made it
    while (simFrame <= gameFrame) {
        // check for inputs made during this frame
        for (size_t i = 0; i < players.size(); ++i) {
            if (inputBuffer[i].count(simFrame) > 0) { // there was an input made by this player during this frame
                players[i].dir = inputBuffer[i][simFrame];
            }
        }

        // resimulate the gametick
        gameTick();
        pastStates[index++] = GameState(simFrame++, players);
    }

    // reset variables
    receivedInputs = 0;
    earliestFrame = -1;
}

void Snake::start() {
    sf::Clock frameTime;
    bool running = true;
    bool paused = false;
    while (running) {
        // increment gameTime
        float delta = frameTime.restart().asSeconds();

        // process window events
        sf::Event e;
        while (window->pollEvent(e)) {
            switch (e.type) {
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::KeyPressed:
                if (e.key.code == sf::Keyboard::Escape) {
                    running = false;
                } else if (e.key.code == sf::Keyboard::Space) {
                    paused = !paused;
                }
            default:
                break;
            }
        }
        if (paused) {
            render();
            continue;
        }

        checkNewConnections();

        checkClientMessages();

        // send packet to clients to update title message
        sf::Packet titlePacket;
        titlePacket << (sf::Uint8) 1;   // message type
        titlePacket << titleVersion++;
        titlePacket << getTitle();
        broadcastPacket(titlePacket);

        unsigned curTime = timeSinceEpochMillis();
        if (curTime > gameStartTime) {
            if (gameFrame == 0 || gameFrame == lastGameFrame) {
                broadcastGameState();
            }

            unsigned curTick = curTime / msPerTick;
            // if enough time has passed then do a game tick
            if (curTick > latestTick) {
                if (receivedInputs > 0) {
                    resimulateGameToPresentState();

                    broadcastGameState();
                }

                latestTick = curTick;
                gameTick();
                gameFrame++;
                std::cout << gameFrame << std::endl;

                pastStates.push_back(GameState(gameFrame, players));
                while (pastStates.size() > pastStateSize) {
                    pastStates.erase(pastStates.begin());
                }
                // should buffer input the same way pastStates are buffered
                for (size_t i = 0; i < inputBuffer.size(); ++i) {
                    int frameToErase = gameFrame - pastStateSize;
                    if (inputBuffer[i].count(frameToErase) > 0) {
                        inputBuffer[i].erase(frameToErase);
                    }
                    //inputBuffer[i].clear();
                }

                winner = getWinner();
                if (winner != 0) {
                    // game will start again in 4000ms
                    gameStartTime = curTime + 4000;
                    lastGameFrame = gameFrame;
                }

                // send game state back to clients
                //broadcastGameState();
            }
        } else if (winner != 0 && gameStartTime - curTime < 1000) {
            resetGame();
            broadcastGameState();
        }

        checkAndSendDelayed();
        checkAndReceiveDelayed();

        // render board and limits framerate
        render();
    }

}

void Snake::resetGame() {
    pastStates.clear();
    std::cout << "Starting new game!" << std::endl;
    winner = 0;

    // reset map
    map.generate();

    // spawn players
    for (size_t i = 0; i < players.size(); ++i) {
        Player& p = players[i];
        switch (p.id) {    // spawn points hardcoded for 2 players for now
        case 0:
            p.spawn(4, 4, 1, 0);
            break;
        case 1:
            p.spawn(map.getW() - 5, map.getH() - 5, -1, 0);
            break;
        default:
            std::cout << "spawning player " << (i + 1) << " at default spawn" << std::endl;
            p.spawn(map.getW() / 2, map.getH() / 2, 1, 0);
            break;
        }
        map.setTile(p.getPos(), p.id + Map::PLAYER);
    }

    foodPos = map.spawnRandom(Map::FOOD);

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

// returns what the title should be based off gametime and if a winner has been found
std::string Snake::getTitle() {
    std::ostringstream oss;
    unsigned gameTime = gameStartTime - timeSinceEpochMillis();

    if (gameStartTime == -1) {      // if waiting for players
        oss << "WAITING...";
    }else if (gameTime < 1000) {    // if game is over then get ready for next!
        oss << "GET READY";
    } else if (gameTime < 2000) {   // if game just ended recently print result
        if (winner < 0) {
            oss << "DRAW!";
        } else {
            oss << "Player " << winner << " wins!";
        }
    } else {    // game is going!
        oss << "SNAKE!";
    }
    return oss.str();
}

void Snake::render() {
    // clear the window to same color as wall tile
    window->clear(sf::Color(12, 26, 51));

    sf::VertexArray verts;
    verts.setPrimitiveType(sf::PrimitiveType::Quads);

    map.generateVertices(verts, TILE);

    window->draw(verts);

    // draw player text (currently hardcoded for two players)
    for (size_t i = 0; i < players.size(); ++i) {
        std::ostringstream oss;

        Player& p = players[i];
        //oss << "P " << (p.id + 1) << " : " << p.score << " ";
        oss << p.playerName << " : " << p.score << " ";
        text.setString(oss.str());
        text.setColor(Map::getColorFromID(p.id + Map::PLAYER));
        // should change this if here to just something like this below

        text.setPosition(sf::Vector2f(800.0f, i * 50.0f + 50.0f));
        window->draw(text);
    }

    // draw title text
    text.setColor(sf::Color(200, 255, 255));
    text.setString(getTitle());
    sf::FloatRect fr = text.getLocalBounds();
    text.setPosition(sf::Vector2f(WIDTH / 2 - fr.width / 2.0f, 0.0f));
    window->draw(text);

    // swap buffers
    window->display();
}


int main() {
    Snake server;
    //std::cout << std::endl << "SERVER STOPPED..." << std::endl;
    //system("pause");
    return 0;
}