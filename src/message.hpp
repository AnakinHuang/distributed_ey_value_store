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

using namespace std;

struct Message {
    string op_type;    // "PUT" or "GET"
    string key;
    string value;
    int lamport_ts{};
    int sender_id{};
    string op_id;

    [[nodiscard]] string serialize() const {
        return op_type + ":" + key + ":" + value + ":" +
               to_string(lamport_ts) + ":" + to_string(sender_id) + ":" + op_id;
    }

    static Message deserialize(const string& data) {
        Message msg;
        size_t pos = 0, prev = 0;
        int count = 0;
        string fields[6];

        while ((pos = data.find(':', prev)) != string::npos && count < 5) {
            fields[count++] = data.substr(prev, pos - prev);
            prev = pos + 1;
        }
        fields[5] = data.substr(prev);

        msg.op_type = fields[0];
        msg.key = fields[1];
        msg.value = fields[2];
        msg.lamport_ts = stoi(fields[3]);
        msg.sender_id = stoi(fields[4]);
        msg.op_id = fields[5];

        return msg;
    }
};

#endif
