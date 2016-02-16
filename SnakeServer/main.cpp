// ICS 167 Multiplayer Snake Project by:
// Matt Ruiz        28465978    mpruiz@uci.edu
// Luke Lohden      23739798    llohden@uci.edu
// John Collins     75665849    jfcollin@uci.edu

#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>
#include <vector>

int main() {
    sf::TcpListener listener;
    listener.setBlocking(false);
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
    if (listener.listen(std::atoi(port.c_str())) != sf::Socket::Done) {
        std::cout << "CANT LISTEN M8" << std::endl;
    }


    std::cout << "SERVER STARTED..." << std::endl;

    int connectedPlayers = 0;
    int maxPlayers = 2;
    // wait for new connection
    std::vector<sf::TcpSocket*> clients;    // remember to delete these! #lol! try using shared_ptrs
    std::vector<int> scores;

    char in[128];
    std::size_t received_len;

    while (true) {
        // push back new potential connection if yes
        if (clients.size() == connectedPlayers) {
            sf::TcpSocket* client = new sf::TcpSocket();
            client->setBlocking(false);
            clients.push_back(client);
            scores.push_back(0);
        }

        if (listener.accept(*clients[connectedPlayers]) != sf::Socket::Done) {
            //std::cout << "CLIENT TIMED OUT MAYBE?" << std::endl;
        } else {
            std::cout << "CLIENT CONNECTED, ASSIGNED AS PLAYER #" << (++connectedPlayers) << std::endl;
            if (connectedPlayers >= 2) {
                std::cout << "GAME STARTING!!!" << std::endl;
                // set game start bool and check down below
            }
        }

        //if (clients.size() < 2) {
        //    continue;
        //}

        for (size_t i = 0, len = clients.size()-1; i < len; ++i) {  // for each player
            sf::Socket::Status status = clients[i]->receive(in, sizeof(in), received_len);

            if (status == sf::Socket::NotReady || status == sf::Socket::Error) {   // no data from the client this frame
                continue;
            } else if (status == sf::Socket::Disconnected) {    // q or disconnected
                std::cout << "PLAYER #" << (i + 1) << " DISCONNECTED! GAME STOPPING..." << std::endl;
                // quit for now but instead later could clear scores and wait for full game to start afresh like before
                goto cya;
            }   // else there was data received!

            // process received data from this player
            // set null character to ignore anything after received length (non blocking sockets get some trash data at end)
            in[received_len] = '\0';
            std::string msg = std::string(in);

            if (msg == "") {
                continue;
            }

            std::cout << "Message from client #" << (i + 1) << " \"" << msg << "\"" << std::endl;

            // empty our message
            int in_len = sizeof(in) / sizeof(char);
            for (int i = 0; i < in_len; ++i) {
                in[i] = '\0';
            }
        }

    }
    cya: // easy game

    system("pause");

    return 0;
}