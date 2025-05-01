#include <cassert>
#include "../src/lamport.hpp"

int main()
{
    LamportClock clock;

    // Initial state
    assert(clock.read() == 0);

    // Local tick increments
    uint64_t t1 = clock.tick();
    assert(t1 == 1);
    uint64_t t2 = clock.tick();
    assert(t2 == 2);

    // Update with higher received timestamp
    uint64_t t3 = clock.update(10);
    // Should be max(received, current) + 1 = 11
    assert(t3 == 11);
    assert(clock.read() == 11);

    // Tick continues from updated time
    uint64_t t4 = clock.tick();
    assert(t4 == 12);

    // Update with lower received timestamp should still increment
    uint64_t t5 = clock.update(5);
    // current is 12, so new_time = 12 + 1 = 13
    assert(t5 == 13);

    return 0;
}
