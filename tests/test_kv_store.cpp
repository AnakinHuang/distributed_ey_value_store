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

#include <cassert>
#include "../src/kv_store.hpp"
#include "../src/message.hpp"

int main()
{
    KVStore store;

    // Initially, no data
    assert(store.get("key1").empty());

    // Create a multicast operation message
    Message msg;
    msg.type = MessageType::MULTICAST_OP;
    msg.op_id = "op1";
    msg.key = "key1";
    msg.value = "value1";

    // Apply operation (pending)
    store.apply(msg);
    // Before commit, get should still be empty
    assert(store.get("key1").empty());

    // Commit operation
    store.commit("op1");
    // After commit, get should return the value
    assert(store.get("key1") == "value1");

    // Committing non-existent op should not break
    store.commit("nonexistent");
    assert(store.get("key1") == "value1");

    // Apply multiple operations
    Message msg2 = msg;
    msg2.op_id = "op2";
    msg2.key = "key2";
    msg2.value = "value2";
    store.apply(msg2);
    // Only op2 pending
    assert(store.get("key2").empty());
    store.commit("op2");
    assert(store.get("key2") == "value2");

    return 0;
}
