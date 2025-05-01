/*
 * File: node.cpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <csignal>
#include "message.hpp"
#include "lamport.hpp"
#include "kv_store.hpp"
#include "network.hpp"

static bool running = true;
// Map operation_id (replica-level) to client info (node_id, client_op_id)
static std::unordered_map<std::string, std::pair<std::string, std::string>> op_client_map;
// Track acknowledgments per operation
static std::unordered_map<std::string, std::unordered_set<std::string>> ack_tracker;
// Track which ops have been committed
static std::unordered_set<std::string> committed_ops;
// Record dynamic quorum size per op_id
static std::unordered_map<std::string, int> op_quorum_size;

void handle_sigint(int)
{
    running = false;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <replica_id> <config_file>\n";
        return 1;
    }
    std::string replica_id = argv[1];
    std::string config_file = argv[2];

    // Networking initialization
    std::vector<std::string> peers;
    int listen_port;
    network::init(replica_id, config_file, peers, listen_port);
    std::signal(SIGINT, handle_sigint);

    LamportClock clock;
    KVStore store;

    while (running)
    {
        Message msg;
        network::receive_message(msg, /*timeout_ms=*/5000);
        std::cout << "[" << replica_id << "] RECEIVED type=" << (int)msg.type
            << " key=" << msg.key << " op=" << msg.op_id << " from="
            << msg.replica_id << "\n";

        switch (msg.type)
        {
        case MessageType::PUT_REQUEST:
            {
                // Capture original client info
                std::string client_node = msg.client_id;
                std::string client_op = msg.op_id;

                // Assign Lamport timestamp and new replica op_id
                uint64_t ts = clock.tick();
                std::string rep_op = replica_id + ":" + std::to_string(ts);

                // Update message for multicast
                msg.type = MessageType::MULTICAST_OP;
                msg.replica_id = replica_id;
                msg.timestamp = ts;
                msg.op_id = rep_op;

                // Record mapping for client ack
                op_client_map[rep_op] = {client_node, client_op};

                // Apply locally so origin also has the update pending
                store.apply(msg);
                // Self-ACK for origin
                ack_tracker[rep_op].insert(replica_id);

                // Dynamic quorum: count live replicas (including self)
                int live_count = 1;

                // Multicast to other peers
                for (const auto& peer : peers)
                {
                    if (network::send_message(peer, msg))
                    {
                        live_count++;
                    }
                    else
                    {
                        std::cerr << "[" << replica_id << "] Peer down, excluding "
                            << peer << " from quorum";
                    }
                }
                op_quorum_size[rep_op] = live_count;
                break;
            }
        case MessageType::MULTICAST_OP:
            {
                // Apply multicast op
                clock.update(msg.timestamp);
                store.apply(msg);

                // Send ACK back to origin
                Message ack;
                ack.type = MessageType::ACK;
                ack.op_id = msg.op_id;
                ack.replica_id = replica_id;
                ack.timestamp = clock.tick();
                std::string origin = network::get_addr(msg.replica_id);
                if (!origin.empty()) network::send_message(origin, ack);
                break;
            }
        case MessageType::ACK:
            {
                // Add ack
                auto& acks = ack_tracker[msg.op_id];
                acks.insert(msg.replica_id);

                // Check dynamic quorum for this op
                int needed = op_quorum_size[msg.op_id] / 2 + 1;
                if (!committed_ops.count(msg.op_id) && (int)acks.size() >= needed)
                {
                    committed_ops.insert(msg.op_id);

                    // Broadcast COMMIT to replicas
                    Message commit;
                    commit.type = MessageType::COMMIT;
                    commit.op_id = msg.op_id;
                    commit.timestamp = clock.tick();
                    for (const auto& peer : peers)
                    {
                        network::send_message(peer, commit);
                    }
                    // Local commit
                    store.commit(msg.op_id);

                    // Send COMMIT-ack to client (using original client op_id)
                    auto it = op_client_map.find(msg.op_id);
                    if (it != op_client_map.end())
                    {
                        const auto& [cnode, cop] = it->second;
                        std::string client_addr = network::get_addr(cnode);
                        Message cack;
                        cack.type = MessageType::COMMIT;
                        cack.op_id = cop;
                        cack.timestamp = clock.tick();
                        std::cout << "[" << replica_id << "] Sending COMMIT to client "
                            << client_addr << " for client_op=" << cop << "\n";
                        if (!client_addr.empty()) network::send_message(client_addr, cack);
                    }
                }
                break;
            }
        case MessageType::COMMIT:
            {
                if (committed_ops.insert(msg.op_id).second)
                {
                    store.commit(msg.op_id);
                }
                break;
            }
        case MessageType::GET_REQUEST:
            {
                // Handle GET
                std::string val = store.get(msg.key);
                Message resp;
                resp.type = MessageType::GET_RESPONSE;
                resp.op_id = msg.op_id;
                resp.key = msg.key;
                resp.value = val;
                resp.client_id = msg.client_id;
                resp.timestamp = clock.tick();
                std::string client_addr = network::get_addr(msg.client_id);
                std::cout << "[" << replica_id << "] Replying GET_RESPONSE to "
                    << client_addr << "='" << val << "'\n";
                if (!client_addr.empty()) network::send_message(client_addr, resp);
                break;
            }
        default:
            std::cerr << "[" << replica_id << "] Unknown message type\n";
        }
    }

    network::shutdown();
    std::cout << "Node " << replica_id << " shutting down.\n";
    return 0;
}
