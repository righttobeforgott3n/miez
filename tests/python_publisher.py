#!/usr/bin/env python3

import ssl
import socket
import sys
import time

DEFAULT_HOST = "localhost"
DEFAULT_PORT = 8443
DEFAULT_CHANNEL = "test-channel"
DEFAULT_NUM_MESSAGES = 10
DEFAULT_API_KEY = None


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


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT
    channel = sys.argv[3] if len(sys.argv) > 3 else DEFAULT_CHANNEL
    num_messages = int(sys.argv[4]) if len(sys.argv) > 4 else DEFAULT_NUM_MESSAGES
    api_key = sys.argv[5] if len(sys.argv) > 5 else DEFAULT_API_KEY
    
    print(f"[Publisher] Connecting to {host}:{port}...")
    
    try:
        sock = create_tls_connection(host, port)
        print(f"[Publisher] Connected!")
        
        if api_key:
            print(f"[Publisher] Authenticating...")
            if not authenticate(sock, api_key):
                print("[Publisher] ERROR: Authentication failed")
                sys.exit(1)
            print(f"[Publisher] Authenticated!")
        
        print(f"[Publisher] Publishing to channel '{channel}'")
        
        for i in range(num_messages):
            content = f"Message #{i+1} from Python publisher at {time.time()}"
            response = publish_message(sock, channel, content)
            print(f"[Publisher] Sent message #{i+1}: {response}")
            time.sleep(1)
        
        response = send_command(sock, "QUIT")
        print(f"[Publisher] Disconnected: {response}")
        
    except ConnectionRefusedError:
        print(f"[Publisher] ERROR: Could not connect to {host}:{port}")
        print("[Publisher] Make sure the server is running.")
        sys.exit(1)
    except Exception as e:
        print(f"[Publisher] ERROR: {e}")
        sys.exit(1)
    finally:
        if 'sock' in locals():
            sock.close()


if __name__ == "__main__":
    main()
