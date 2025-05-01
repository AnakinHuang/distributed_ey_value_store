/*
 * File: client.cpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: Fix for sequential coordinator selection
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <csignal>
#include <cstdint>
#include "message.hpp"
#include "network.hpp"

static bool running = true;
static uint64_t get_counter = 0;
static uint64_t put_counter = 0;

void handle_sigint(int) { running = false; }

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <client_id> <config_file>\n";
        return 1;
    }
    std::string client_id = argv[1];
    std::string config_file = argv[2];

    // Bind client listener and load all replica addresses
    std::vector<std::string> peers;
    int listen_port;
    network::init(client_id, config_file, peers, listen_port);

    std::signal(SIGINT, handle_sigint);
    std::cout << "Commands: put <key> <value> | get <key> | exit\n";

    while (running)
    {
        std::cout << ">> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "put")
        {
            // ---- PUT branch ----
            std::string key, value;
            iss >> key >> value;
            if (key.empty() || value.empty())
            {
                std::cerr << "Usage: put <key> <value>\n";
                continue;
            }

            // Construct the PUT request
            Message msg;
            msg.type = MessageType::PUT_REQUEST;
            msg.key = key;
            msg.value = value;
            msg.client_id = client_id;
            msg.op_id = client_id + ":" + std::to_string(++put_counter);

            // Sequential fail-over: try one replica at a time
            bool sent = false;
            for (auto& peer : peers)
            {
                if (network::send_message(peer, msg))
                {
                    std::cout << "PUT request broadcast: (" << key << ", " << value << ")\n";
                    sent = true;
                    break;
                }
                // on connect failure, try next peer
            }
            if (!sent)
            {
                std::cerr << "PUT failed: no live replicas\n";
                continue;
            }

            // Wait for exactly one COMMIT for our op_id
            while (running)
            {
                Message resp;
                if (network::receive_message(resp, /*timeout_ms=*/5000)
                    && resp.type == MessageType::COMMIT
                    && resp.op_id == msg.op_id)
                {
                    break;
                }
            }
        }
        else if (cmd == "get")
        {
            // ---- GET branch ----
            std::string key;
            iss >> key;
            if (key.empty())
            {
                std::cerr << "Usage: get <key>\n";
                continue;
            }
            // Construct the GET request
            Message msg;
            msg.type = MessageType::GET_REQUEST;
            msg.key = key;
            msg.client_id = client_id;
            msg.op_id = client_id + ":" + std::to_string(++get_counter);

            // Sequential fail-over: one replica at a time
            bool sent = false;
            for (auto& peer : peers)
            {
                if (network::send_message(peer, msg))
                {
                    sent = true;
                    break;
                }
            }
            if (!sent)
            {
                std::cerr << "GET failed: no live replicas\n";
                continue;
            }

            // Wait for exactly one GET_RESPONSE for our op_id
            while (running)
            {
                Message resp;
                if (network::receive_message(resp, /*timeout_ms=*/5000)
                    && resp.type == MessageType::GET_RESPONSE
                    && resp.op_id == msg.op_id)
                {
                    std::cout << "GET response: " << resp.value << "\n";
                    break;
                }
            }
        }
        else if (cmd == "exit")
        {
            break;
        }
        else
        {
            std::cerr << "Unknown command: " << cmd << "\n";
        }
    }

    network::shutdown();
    std::cout << "Client shutting down.\n";
    return 0;
}
