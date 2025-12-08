# miez

An educational purpose driven, lightweight, thread-safe message broker written in C with TLS support.

**Note:** This project is intended for educational purposes. It demonstrates the implementation of fundamental data structures, thread synchronization, and the publish/subscribe pattern from scratch in C. The network server and `main.c` are quick demos to showcase the broker's capabilities, not production-ready interfaces.

## Description

miez is a publish/subscribe message broker that enables asynchronous communication between distributed applications. It provides:

- **Channel-based routing**: Messages are published to named channels and delivered to all subscribers of that channel
- **Thread-safe operations**: All data structures and broker operations are designed for concurrent access
- **Persistent subscriptions**: Subscribers can disconnect and reconnect without losing messages
- **Mailbox pattern**: Each subscriber has a dedicated inbox queue, ensuring no message loss during temporary disconnections

> The name "miez" comes from Neapolitan dialect and means "in the middle" – exactly where a message broker sits to do its job: in the middle between applications that want to communicate.

### Components

The project includes several reusable generic data structures:

- **generic_linked_list**: Doubly linked list with iterator support and configurable ownership
- **generic_queue / generic_queue_syn**: FIFO queue with thread-safe variant
- **generic_hash_table**: Hash table with per-bucket locking for concurrent access
- **thread_pool**: Worker thread pool for async task execution

## Requirements

### System

- POSIX-compliant operating system (Linux, macOS, BSD)
- C17 compatible compiler
- CMake 3.10 or higher

### Dependencies

- **pthread**: POSIX Threads (usually included with libc)
- **OpenSSL**: For TLS support

On Debian/Ubuntu:
```bash
sudo apt-get install build-essential clang cmake libssl-dev
```

On Fedora/RHEL:
```bash
sudo dnf install clang cmake openssl-devel
```

## Building (for devs)

```bash
# Clone the repository
git clone https://github.com/righttobeforgott3n/miez.git
cd miez

cmake -S . -B build -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang -DCMAKE_BUILD_TYPE:STRING=Debug -DENABLE_TESTS=1
cmake --build build
```

## Usage

### Generating TLS Certificates

Before running the server, generate self-signed certificates for development:

```bash
cd tests
./tests/scripts/generate_certs.sh
```

This creates `server.crt`, `server.key`, and `server.pem` in the `./tests/certs/` directory.

### Starting the Server

```bash
./build/bin/network_server [options]
```

**Options:**

| Option | Description | Default |
|--------|-------------|---------|
| `-p <port>` | Port to listen on | 8443 |
| `-c <cert>` | Path to certificate file | certs/server.crt |
| `-k <key>` | Path to private key file | certs/server.key |
| `-a <api_key>` | API key for authentication | (none) |
| `-t <threads>` | Number of broker threads | 4 |
| `-h` | Show help message | - |

**Example:**

```bash
./build/bin/network_server -p 8443 -c ./tests/certs/server.crt -k ./tests/certs/server.key -a my-secret-key
```

### Protocol

miez uses a simple text-based protocol over TLS (It is just a demo, not to be intended as the final networking interface or protocol):

| Command | Syntax | Response | Description |
|---------|--------|----------|-------------|
| AUTH | `AUTH <api_key>` | `OK` / `ERR Invalid API key` | Authenticate client |
| SUBSCRIBE | `SUBSCRIBE <channel>` | `OK <subscription_id>` | Subscribe to a channel |
| PUBLISH | `PUBLISH <channel> <len>\n<content>` | `OK <msg_id> <subscribers>` | Publish a message |
| DETACH | `DETACH` | `OK <subscription_id>` | Disconnect but keep subscription alive |
| ATTACH | `ATTACH <subscription_id>` | `OK <pending_count>` | Reconnect to existing subscription |
| QUIT | `QUIT` | `BYE` | Disconnect |

**Message format (received by subscribers):**
```
MSG <msg_id> <channel> <content_len>
<content>
```

### Python Client Examples

The `tests/` directory includes also some Python client examples:

**Subscriber:**
```bash
python3 tests/python_subscriber.py localhost 8443 my-channel 60 my-secret-key
```

**Publisher:**
```bash
python3 tests/python_publisher.py localhost 8443 my-channel 10 my-secret-key
```

**Reconnection test:**
```bash
python3 tests/test_reconnection.py localhost 8443 my-secret-key
```

### Using the Broker Programmatically (C API)

```c
#include "message_broker.h"

// Create broker
struct message_broker_configuration_t config = {
    ._n_threads = 4,
    ._channels_capacity = 64
};
struct message_broker_t* broker;
message_broker_new(&config, &broker);

// Subscribe
struct subscription_t* sub;
message_broker_subscribe(broker, "my-channel", &sub);

// Publish
message_broker_publish(broker, "my-channel", "Hello, World!");

// Receive (blocking)
struct message_t* msg;
subscription_receive(sub, &msg);

const char* content;
message_get_content(msg, &content);
printf("Received: %s\n", content);

message_free(msg);

// Cleanup
subscription_unsubscribe(sub);
subscription_free(sub);
message_broker_free(broker);
```

## Limitations

- **POSIX only**: Uses POSIX APIs (pthread, sockets); not compatible with Windows
- **No message acknowledgment**: Messages are removed from queue on dequeue without delivery confirmation
- **No message TTL**: Messages don't expire; risk of memory exhaustion for slow consumers, and generally speaking eviction policies aren't implemented.
- **No persistence**: All data is in-memory and lost on restart
- **Single node**: No clustering or replication support
- **Global authentication**: Single API key for all clients; no per-channel permissions

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
