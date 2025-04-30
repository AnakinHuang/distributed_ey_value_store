/*
 * File: test_kv_store.cpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
*/
#include <cassert>
#include <iostream>
#include "../src/kv_store.hpp"
#include "../src/message.hpp"

int main() {
    KVStore store;

    // Prepare a MULTICAST_OP message
    Message msg;
    msg.type = MessageType::MULTICAST_OP;
    msg.key = "foo";
    msg.value = "bar";
    msg.timestamp = 1;
    msg.client_id = "client1";
    msg.replica_id = "replica1";
    msg.op_id = "op1";

    // Applying should not immediately update the store
    store.apply(msg);
    assert(store.get("foo").empty());

    // Committing should make the value available
    store.commit("op1");
    assert(store.get("foo") == "bar");

    // Committing a non-existent op should be a no-op
    store.commit("nonexistent");
    assert(store.get("foo") == "bar");

    std::cout << "[PASS] KVStore apply/commit tests" << std::endl;
    return 0;
}
