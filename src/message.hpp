/*
 * File: message.hpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
*/
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <sstream>
#include <cstdint>

// Types of messages exchanged between client and replicas
enum class MessageType
{
    PUT_REQUEST = 0,
    GET_REQUEST,
    MULTICAST_OP,
    ACK,
    COMMIT,
    GET_RESPONSE
};

// Generic message struct with basic serialization/deserialization
struct Message
{
    MessageType type;
    std::string key;
    std::string value;
    uint64_t timestamp; // Lamport timestamp or logical time
    std::string client_id; // Identifier for the client
    std::string replica_id; // Identifier for the replica (for ACKs)
    std::string op_id; // Unique operation ID

    // Serialize to a delimited string
    std::string serialize() const
    {
        std::ostringstream oss;
        oss << static_cast<int>(type) << '|' // message type
            << key << '|' // key
            << value << '|' // value
            << timestamp << '|' // timestamp
            << client_id << '|' // client ID
            << replica_id << '|' // replica ID
            << op_id; // operation ID
        return oss.str();
    }

    // Deserialize from a delimited string
    static Message deserialize(const std::string& data)
    {
        Message msg;
        std::istringstream iss(data);
        std::string token;

        std::getline(iss, token, '|');
        msg.type = static_cast<MessageType>(std::stoi(token));
        std::getline(iss, msg.key, '|');
        std::getline(iss, msg.value, '|');
        std::getline(iss, token, '|');
        msg.timestamp = std::stoull(token);
        std::getline(iss, msg.client_id, '|');
        std::getline(iss, msg.replica_id, '|');
        std::getline(iss, msg.op_id, '|');

        return msg;
    }
};

#endif // MESSAGE_HPP
