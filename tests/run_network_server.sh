#!/bin/bash
cd "$(dirname "$0")/.."
./build/bin/network_server -p 8443 -c ./tests/certs/server.crt -k ./tests/certs/server.key -a shh... 2>&1


# To test the server:

# Subscriber
# python3 tests/python_subscriber.py localhost 8443 test-channel 60 your-secret-key

# Publisher  
# python3 tests/python_publisher.py localhost 8443 test-channel 5 your-secret-key

# Reconnection test (it uses both a sub and pub in an automatic way)
# python3 tests/test_reconnection.py localhost 8443 your-secret-key