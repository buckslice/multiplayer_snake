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

    // wait for new connection
    sf::TcpSocket client;
    if (listener.accept(client) != sf::Socket::Done) {
        std::cout << "NO CLIENT" << std::endl;
    } else {
        std::cout << "CLIENT CONNECTED!" << std::endl;
    }

    char in[128];
    std::size_t received_len;

    // later find number of players
    std::vector<int> scores(2);

    while (true) {
        if (client.receive(in, sizeof(in), received_len) != sf::Socket::Done) {
            std::cout << "CLIENT DISCONNECTED!" << std::endl;
            // clear scores
            for (size_t i = 0; i < scores.size(); ++i) {
                scores[i] = 0;
            }

            // if client disconnected then wait for a new connection
            //return 0; // could also just quit once client disconnects
            if (listener.accept(client) != sf::Socket::Done) {
                std::cout << "NO CLIENT" << std::endl;
            } else {
                std::cout << "CLIENT CONNECTED!" << std::endl;
            }
        }

        // set null character to ignore anything after received length
        in[received_len] = '\0';
        std::string msg = std::string(in);

        if (msg == "") {
            continue;
        } else if (msg == "0") {
            // clear scores
            for (size_t i = 0; i < scores.size(); ++i) {
                scores[i] = 0;
            }
        } else {
            // add one to players score
            int player = std::atoi(msg.c_str()) - 1;
            scores[player]++;

            // send back score vector
            std::ostringstream oss;
            for (size_t i = 0; i < scores.size(); ++i) {
                oss << scores[i];
            }

            std::string s = oss.str();
            client.send(s.c_str(), s.length());
        }

        std::cout << "Message from client: \"" << in << "\"" << std::endl;

        // empty our message
        int in_len = sizeof(in) / sizeof(char);
        for (int i = 0; i < in_len; ++i) {
            in[i] = '\0';
        }

    }

    system("pause");

    return 0;
}