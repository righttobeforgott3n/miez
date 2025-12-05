#!/usr/bin/env python3

import ssl
import socket
import sys
import time
import threading

DEFAULT_HOST = "localhost"
DEFAULT_PORT = 8443
DEFAULT_API_KEY = None
TEST_CHANNEL = "reconnect-test"


def create_tls_connection(host: str, port: int) -> ssl.SSLSocket:
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    ssl_sock = context.wrap_socket(sock, server_hostname=host)
    ssl_sock.connect((host, port))
    
    return ssl_sock


def send_command(sock: ssl.SSLSocket, command: str) -> str:
    sock.send((command + "\n").encode())
    response = b""
    while not response.endswith(b"\n"):
        chunk = sock.recv(1)
        if not chunk:
            break
        response += chunk
    return response.decode().strip()


def authenticate(sock: ssl.SSLSocket, api_key: str) -> bool:
    if not api_key:
        return True
    response = send_command(sock, f"AUTH {api_key}")
    return response == "OK"


def publish_message(sock: ssl.SSLSocket, channel: str, content: str) -> str:
    content_bytes = content.encode()
    command = f"PUBLISH {channel} {len(content_bytes)}"
    sock.send((command + "\n").encode())
    sock.send(content_bytes + b"\n")
    
    response = b""
    while not response.endswith(b"\n"):
        chunk = sock.recv(1)
        if not chunk:
            break
        response += chunk
    return response.decode().strip()


def read_line(sock: ssl.SSLSocket, timeout: float = 5.0) -> str:
    sock.settimeout(timeout)
    line = b""
    try:
        while not line.endswith(b"\n"):
            chunk = sock.recv(1)
            if not chunk:
                return None
            line += chunk
        return line.decode().strip()
    except socket.timeout:
        return None


def receive_all_pending_messages(sock: ssl.SSLSocket, timeout: float = 2.0) -> list:
    messages = []
    while True:
        header = read_line(sock, timeout)
        if header is None:
            break
        
        if header.startswith("MSG "):
            parts = header.split(" ")
            if len(parts) >= 4:
                msg_id = parts[1]
                channel = parts[2]
                content_len = int(parts[3])
                
                sock.settimeout(timeout)
                content = b""
                while len(content) < content_len:
                    chunk = sock.recv(content_len - len(content))
                    if not chunk:
                        break
                    content += chunk
                sock.recv(1)
                
                messages.append({
                    'id': msg_id,
                    'channel': channel,
                    'content': content.decode()
                })
    return messages


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT
    api_key = sys.argv[3] if len(sys.argv) > 3 else DEFAULT_API_KEY
    
    print("=" * 60)
    print("MESSAGE BROKER RECONNECTION TEST")
    print("=" * 60)
    print(f"Server: {host}:{port}")
    print(f"Channel: {TEST_CHANNEL}")
    print(f"API Key: {'********' if api_key else '(none)'}")
    print("=" * 60)
    
    print("\n[STEP 1] Subscriber connects and subscribes...")
    sub_sock = create_tls_connection(host, port)
    
    if api_key:
        if not authenticate(sub_sock, api_key):
            print("  ERROR: Authentication failed")
            sys.exit(1)
        print("  Authenticated!")
    
    response = send_command(sub_sock, f"SUBSCRIBE {TEST_CHANNEL}")
    print(f"  Subscribe response: {response}")
    
    if not response.startswith("OK"):
        print("  ERROR: Failed to subscribe")
        sys.exit(1)
    
    subscription_id = response.split()[1]
    print(f"  Subscription ID: {subscription_id}")
    
    time.sleep(0.5)
    
    print("\n[STEP 2] Subscriber detaches (keeps subscription alive)...")
    response = send_command(sub_sock, "DETACH")
    print(f"  Detach response: {response}")
    
    if not response.startswith("OK"):
        print("  ERROR: Failed to detach")
        sys.exit(1)
    
    sub_sock.close()
    print("  Subscriber disconnected")
    
    print("\n[STEP 3] Publisher publishes messages while subscriber is offline...")
    pub_sock = create_tls_connection(host, port)
    
    if api_key:
        if not authenticate(pub_sock, api_key):
            print("  ERROR: Authentication failed")
            sys.exit(1)
        print("  Authenticated!")
    
    num_messages = 5
    for i in range(num_messages):
        content = f"Offline message #{i+1} - timestamp {time.time()}"
        response = publish_message(pub_sock, TEST_CHANNEL, content)
        print(f"  Published message #{i+1}: {response}")
        time.sleep(0.2)
    
    send_command(pub_sock, "QUIT")
    pub_sock.close()
    print("  Publisher disconnected")
    
    print("\n[STEP 4] Simulating offline period (2 seconds)...")
    time.sleep(2)
    
    print("\n[STEP 5] Subscriber reconnects using ATTACH...")
    sub_sock = create_tls_connection(host, port)
    
    if api_key:
        if not authenticate(sub_sock, api_key):
            print("  ERROR: Authentication failed")
            sys.exit(1)
        print("  Authenticated!")
    
    response = send_command(sub_sock, f"ATTACH {subscription_id}")
    print(f"  Attach response: {response}")
    
    if not response.startswith("OK"):
        print("  ERROR: Failed to attach")
        sys.exit(1)
    
    pending_count = int(response.split()[1])
    print(f"  Pending messages: {pending_count}")
    
    print("\n[STEP 6] Receiving pending messages...")
    messages = receive_all_pending_messages(sub_sock, timeout=2.0)
    
    for msg in messages:
        print(f"  Message ID {msg['id']}: {msg['content']}")
    
    send_command(sub_sock, "QUIT")
    sub_sock.close()
    
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    print(f"Messages published while offline: {num_messages}")
    print(f"Messages received after reconnect: {len(messages)}")
    
    if len(messages) == num_messages:
        print("\n✓ TEST PASSED: All messages received!")
        return 0
    else:
        print(f"\n✗ TEST FAILED: Expected {num_messages}, got {len(messages)}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
