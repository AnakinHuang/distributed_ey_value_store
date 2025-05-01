CSC458 Final Project: Distributed Key-Value Store
=================================================

Author
  Yuesong Huang (Anakin Huang)
  NetID: yhu116
  Email: yhu116@u.rochester.edu

Overview
  A simple replicated key-value store supporting PUT/GET with total-order multicast
  and stop-fault tolerance via majority voting. Uses Lamport logical clocks for
  ordering and TCP for inter-node messaging.

Repo Layout
  csc458_final_project/
  ├── src/
  │   ├── node.cpp           # replica process
  │   ├── client.cpp         # interactive client
  │   ├── network.hpp/.cpp   # TCP networking + listener thread
  │   ├── lamport.hpp/.cpp   # LamportClock
  │   ├── kv_store.hpp/.cpp  # KVStore logic
  │   └── message.hpp        # Message struct + (de)serialization
  ├── tests/
  │   ├── test_lamport.cpp   # unit tests for LamportClock
  │   └── test_kv_store.cpp  # unit tests for KVStore
  ├── client_config.txt      # sample config (A,B,C,client1)
  ├── Makefile
  ├── eval.sh            # smoke‐test & micro‐benchmark script
  ├── logs/                  # smoke‐test logs
  ├── benchmarks/            # benchmark CSVs
  └── README.txt

Prerequisites
  • Linux or macOS
  • g++ with C++20 support
  • pthreads
  • Bash shell

Build
  cd csc458_final_project
  make all

Binaries produced
  • node            # replica
  • client          # interactive client
  • test_lamport
  • test_kv_store

Configuration
  Edit (or use) client_config.txt to list each node/client:
    A       127.0.0.1 5030
    B       127.0.0.1 5031
    C       127.0.0.1 5032
    client1 127.0.0.1 5999

Running a 3-node cluster + client
  # In three terminals:
  ./node A client_config.txt
  ./node B client_config.txt
  ./node C client_config.txt

  # In a fourth terminal:
  ./client client1 client_config.txt
  Commands: put <key> <value> | get <key> | exit

  Example:
    >> put x 42
    PUT request sent to 127.0.0.1:5030: (x, 42)
    PUT committed: x

    >> get x
    GET response: 42

Fault Tolerance Test
  1. Start A, B, C.
  2. In client do a few PUT/GET.
  3. Kill one replica (Ctrl-C).
  4. Verify new PUT/GET still succeed via the remaining two.

Unit Tests
  make tests
  ./test_lamport
  ./test_kv_store

Smoke‐Test & Benchmark Script
  A combined script `run_eval.sh` automates both correctness smoke‐tests
  and a simple micro‐benchmark of PUT throughput. To run:

    chmod +x run_eval.sh
    ./run_eval.sh

  It will:
    • Start replicas A, B, C
    • Perform:
        – cold GET
        – basic PUT→GET
        – one-node-down PUT→GET
    • Report PASS/FAIL for each smoke test, with logs in `logs/`
    • Restart node C, then run 1 000 back-to-back PUTs:
        – “healthy” (all replicas up)
        – “one-down” (with one replica killed)
    • Output a CSV `benchmarks/put_throughput.csv`:
        mode,ops,total_us,throughput_ops_per_sec
    • Print summary throughput stats on-screen.

Known Limitations
  • No automated GET benchmark—only PUT is measured.
  • No leader election; each PUT is coordinated by the first live replica.
  • No state re-sync for nodes that rejoin after failure.

Future Work
  • Expand the script to benchmark GET latency and concurrency.
  • Integrate persistent storage and recovery.
  • Add batching/pipelining for higher throughput.
  • Implement dynamic leader election for load-balancing.
