#include "network.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace network
{
    static std::unordered_map<std::string, std::string> id_addr_map;
    static int listen_sock = -1;
    static std::vector<std::string> peers;

    static std::thread listener_thread;
    static std::mutex mtx;
    static std::condition_variable cv;
    static std::queue<Message> msg_queue;
    static bool running = false;

    // Helper: split "host:port"
    static void split_host_port(const std::string& hp, std::string& host, int& port)
    {
        size_t pos = hp.find(':');
        host = hp.substr(0, pos);
        port = std::stoi(hp.substr(pos + 1));
    }

    void init(const std::string& node_id,
              const std::string& config_file,
              std::vector<std::string>& out_peers,
              int& out_port)
    {
        id_addr_map.clear();
        out_peers.clear();

        // Read config
        std::ifstream ifs(config_file);
        if (!ifs.is_open())
        {
            std::cerr << "Failed to open config file: " << config_file << "\n";
            std::exit(1);
        }
        std::string line;
        while (std::getline(ifs, line))
        {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string id, host;
            int port;
            iss >> id >> host >> port;
            std::string addr = host + ":" + std::to_string(port);
            id_addr_map[id] = addr;
        }

        // Determine self listen port
        auto self_it = id_addr_map.find(node_id);
        if (self_it == id_addr_map.end())
        {
            std::cerr << "Node ID not found in config: " << node_id << "\n";
            std::exit(1);
        }
        {
            std::string host_self;
            int port_self;
            split_host_port(self_it->second, host_self, port_self);
            out_port = port_self;
        }

        // Populate peers (addresses) excluding self
        for (auto& kv : id_addr_map)
        {
            if (kv.first == node_id) continue;
            out_peers.push_back(kv.second);
        }
        peers = out_peers;

        // Create listening socket
        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0)
        {
            perror("socket");
            std::exit(1);
        }
        int opt = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(out_port);

        if (bind(listen_sock, (sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            std::exit(1);
        }
        if (listen(listen_sock, 10) < 0)
        {
            perror("listen");
            std::exit(1);
        }

        running = true;
        listener_thread = std::thread([]()
        {
            while (running)
            {
                sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                int client_fd = accept(listen_sock, (sockaddr*)&cli_addr, &cli_len);
                if (client_fd < 0)
                {
                    if (!running) break;
                    perror("accept");
                    continue;
                }
                std::string data;
                char ch;
                while (read(client_fd, &ch, 1) == 1)
                {
                    if (ch == '\n') break;
                    data.push_back(ch);
                }
                close(client_fd);
                if (!data.empty())
                {
                    Message msg = Message::deserialize(data);
                    std::unique_lock<std::mutex> lock(mtx);
                    msg_queue.push(msg);
                    lock.unlock();
                    cv.notify_one();
                }
            }
        });
    }

    std::string get_addr(const std::string& node_id)
    {
        auto it = id_addr_map.find(node_id);
        return (it == id_addr_map.end() ? std::string() : it->second);
    }

    bool send_message(const std::string& dest_addr, const Message& msg)
    {
        std::string host;
        int port;
        split_host_port(dest_addr, host, port);
        addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0)
        {
            perror("getaddrinfo");
            return false;
        }
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            perror("socket");
            freeaddrinfo(res);
            return false;
        }
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            perror("connect");
            close(sock);
            freeaddrinfo(res);
            return false;
        }
        freeaddrinfo(res);
        std::string out = msg.serialize() + "\n";
        if (write(sock, out.c_str(), out.size()) < 0)
        {
            perror("write");
            close(sock);
            return false;
        }
        close(sock);
        return true;
    }

    bool receive_message(Message& msg, int timeout_ms)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [] { return !msg_queue.empty(); }))
        {
            return false; // timeout
        }
        msg = msg_queue.front();
        msg_queue.pop();
        return true;
    }

    void shutdown()
    {
        running = false;
        if (listen_sock >= 0) close(listen_sock);
        if (listener_thread.joinable()) listener_thread.join();
    }
} // namespace network
