// network.hpp
// Created by Yuesong Huang on 4/30/25.

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include "message.hpp"

// Simple TCP networking layer for one-to-one messaging among replicas/clients
namespace network
{
    /**
     * Initialize networking: parse config, start listener
     * config_file format: one entry per line: <node_id> <host> <port>
     * On return,
     *  - peers contains "host:port" addresses of all other nodes
     *  - listen_port is the TCP port this node listens on
     */
    void init(const std::string& node_id,
              const std::string& config_file,
              std::vector<std::string>& peers,
              int& listen_port);

    /**
     * Lookup the "host:port" address for a given node ID (replica or client)
     * Requires init() to have been called.
     * Returns empty string if ID not found.
     */
    std::string get_addr(const std::string& node_id);

    /**
     * Send a Message to the destination address "host:port".
     * This is a one-shot TCP connect/send/close.
     */
    bool send_message(const std::string& dest_addr, const Message& msg);

    /**
     * Blocking receive: waits until a message arrives in the incoming queue.
     * Messages are deserialized before being queued.
     */
    bool receive_message(Message& msg, int timeout_ms);

    /**
     * Shutdown networking: stops listener thread and closes sockets.
     */
    void shutdown();
} // namespace network

#endif // NETWORK_HPP
