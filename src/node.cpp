/*
 * File: node.cpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
*/
#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "message.hpp"

using namespace std;

constexpr int BUFFER_SIZE = 1024;

void start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    cout << "[Server] Listening on port " << port << endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

        char buffer[BUFFER_SIZE] = {0};
        read(client_fd, buffer, BUFFER_SIZE);
        Message msg = Message::deserialize(buffer);
        cout << "[Server] Received: " << buffer << endl;
        close(client_fd);
    }
}

void send_message(const string& ip, int port, const Message& msg) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);

    connect(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    string serialized = msg.serialize();
    send(sock_fd, serialized.c_str(), serialized.size(), 0);
    cout << "[Client] Sent: " << serialized << endl;
    close(sock_fd);
}

int main(const int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: ./node <node_id> <listen_port> <target_port>\n";
        return 1;
    }

    const int node_id = stoi(argv[1]);
    int listen_port = stoi(argv[2]);
    const int target_port = stoi(argv[3]);

    thread server_thread(start_server, listen_port);

    // Allow server to start
    sleep(1);

    // Example: send a PUT message to the other node
    Message msg;
    msg.op_type = "PUT";
    msg.key = "x";
    msg.value = "42";
    msg.lamport_ts = 1;
    msg.sender_id = node_id;
    msg.op_id = "op123";

    send_message("127.0.0.1", target_port, msg);

    server_thread.join();
    return 0;
}
