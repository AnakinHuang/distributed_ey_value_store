# Distributed Key-Value Store

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)

## 📖 Overview

This project implements a simple replicated key-value store supporting `PUT` and `GET` operations with total-order multicast and stop-fault tolerance via majority voting. It utilizes Lamport logical clocks for ordering and TCP for inter-node messaging.

---

## 🧩 Features

- **Total-Order Multicast**: Ensures that all messages are delivered in the same order across all nodes using Lamport clocks.
- **Fault Tolerance**: Achieves stop-fault tolerance through majority voting among replicas.
- **Client-Server Architecture**: Interactive client communicates with replica nodes over TCP.
- **Concurrency**: Handles multiple client requests concurrently with thread-safe operations.

---

## 🛠️ Architecture

The system comprises the following components:

- **Replica Node (`node.cpp`)**: Each replica maintains a local key-value store and participates in the consensus protocol.
- **Client (`client.cpp`)**: Sends `PUT` and `GET` requests to the replicas and displays responses.
- **Networking (`network.hpp/.cpp`)**: Manages TCP connections and message passing between clients and replicas.
- **Lamport Clock (`lamport.hpp/.cpp`)**: Implements Lamport logical clocks to order events in the distributed system.
- **Key-Value Store (`kv_store.hpp/.cpp`)**: Provides thread-safe operations for storing and retrieving key-value pairs.

---

## 📂 Directory Structure

```
distributed_key_value_store/
├── src/
│   ├── node.cpp           # Replica process
│   ├── client.cpp         # Interactive client
│   ├── network.hpp/.cpp   # TCP networking and listener thread
│   ├── lamport.hpp/.cpp   # Lamport clock implementation
│   ├── kv_store.hpp/.cpp  # Key-value store logic
├── tests/                 # Test cases
├── CMakeLists.txt         # Build configuration
├── Makefile               # Alternative build configuration
├── LICENSE                # MIT License
├── README.md              # Project documentation
```

---

## 🚀 Getting Started

### Prerequisites

- **C++ Compiler**: Ensure you have a C++ compiler that supports C++11 or later.
- **CMake**: Version 3.10 or higher.

### Installation

1. **Clone the repository:**

   ```bash
   git clone https://github.com/AnakinHuang/distributed_key_value_store.git
   cd distributed_key_value_store
   ```

2. **Build the project:**

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

### Usage

1. **Start Replica Nodes:**

   In separate terminals, start multiple replica nodes:

   ```bash
   ./node <node_id> <config_file>
   ```

   Replace `<node_id>` with a unique identifier for the node and `<config_file>` with the path to the configuration file.

2. **Start Client:**

   ```bash
   ./client <config_file>
   ```

   The client will connect to the replicas as specified in the configuration file.

3. **Perform Operations:**

   Once the client is running, you can perform `PUT` and `GET` operations:

   ```
   PUT <key> <value>
   GET <key>
   ```

---

## ✅ Testing

To run the test cases:

```bash
cd tests
./run_tests.sh
```

Ensure all tests pass to verify the correctness of the implementation.

---

## 👥 Contributors

- **Yuesong Huang (Anakin Huang)** – [yhu116@u.rochester.edu](mailto:yhu116@u.rochester.edu)

---

## 📄 License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

---
