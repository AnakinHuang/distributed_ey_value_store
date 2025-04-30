/*
 * File: kv_store.hpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
*/
#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include <string>
#include <unordered_map>
#include "message.hpp"

// In-memory key-value store with operation logging and commit semantics
class KVStore {
public:
    KVStore() = default;

    // Apply an operation (store it pending commit)
    void apply(const Message& msg) {
        // Only apply PUT operations
        if (msg.type == MessageType::MULTICAST_OP) {
            pending_[msg.op_id] = msg;
        }
    }

    // Commit a previously applied operation by op_id
    void commit(const std::string& op_id) {
        auto it = pending_.find(op_id);
        if (it != pending_.end()) {
            // Update the actual store
            data_[it->second.key] = it->second.value;
            // Remove from pending log
            pending_.erase(it);
        }
    }

    // Read a value for a given key (empty string if not found)
    std::string get(const std::string& key) const {
        auto it = data_.find(key);
        return (it != data_.end() ? it->second : std::string());
    }

private:
    // Committed key-value data
    std::unordered_map<std::string, std::string> data_;
    // Operations received but awaiting commit
    std::unordered_map<std::string, Message> pending_;
};

#endif // KV_STORE_HPP

