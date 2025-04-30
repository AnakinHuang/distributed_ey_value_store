/*
 * File: lamport.hpp
 * Creator: Yuesong Huang
 * Email (NetID): yhu116@u.rochester.edu
 * Date: 2025/4/30
 * Contributor: N/A
*/
#ifndef LAMPORT_HPP
#define LAMPORT_HPP

#include <atomic>
#include <cstdint>

// Thread-safe Lamport logical clock
class LamportClock {
public:
    LamportClock() : counter(0) {}

    // Get the next timestamp for a local event
    uint64_t tick() {
        return ++counter;
    }

    // Update the clock based on a received timestamp and return the new time
    uint64_t update(uint64_t received) {
        uint64_t current = counter.load();
        uint64_t new_time = (received > current ? received : current) + 1;
        counter.store(new_time);
        return new_time;
    }

    // Peek at the current clock value
    uint64_t read() const {
        return counter.load();
    }

private:
    std::atomic<uint64_t> counter;
};

#endif // LAMPORT_HPP
