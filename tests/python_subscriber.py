#!/usr/bin/env python3

import ssl
import socket
import sys
import time
import threading

DEFAULT_HOST = "localhost"
DEFAULT_PORT = 8443
DEFAULT_CHANNEL = "test-channel"
DEFAULT_TIMEOUT = 30
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


def read_line(sock: ssl.SSLSocket) -> str:
    line = b""
    while not line.endswith(b"\n"):
        chunk = sock.recv(1)
        if not chunk:
            return None
        line += chunk
    return line.decode().strip()


def receive_messages(sock: ssl.SSLSocket, stop_event: threading.Event):
    message_count = 0
    
    while not stop_event.is_set():
        try:
            sock.settimeout(1.0)
            
            header = read_line(sock)
            if header is None:
                print("[Subscriber] Connection closed by server")
                break
            
            if header.startswith("MSG "):
                parts = header.split(" ")
                if len(parts) >= 4:
                    msg_id = parts[1]
                    channel = parts[2]
                    content_len = int(parts[3])
                    
                    content = b""
                    while len(content) < content_len:
                        chunk = sock.recv(content_len - len(content))
                        if not chunk:
                            break
                        content += chunk
                    
                    sock.recv(1)
                    
                    message_count += 1
                    print(f"[Subscriber] Message #{message_count} (ID: {msg_id}) from '{channel}':")
                    print(f"             Content: {content.decode()}")
            else:
                print(f"[Subscriber] Received: {header}")
                
        except socket.timeout:
            continue
        except Exception as e:
            if not stop_event.is_set():
                print(f"[Subscriber] Error receiving: {e}")
            break
    
    return message_count


def main():
    host = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_HOST
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT
    channel = sys.argv[3] if len(sys.argv) > 3 else DEFAULT_CHANNEL
    timeout = int(sys.argv[4]) if len(sys.argv) > 4 else DEFAULT_TIMEOUT
    api_key = sys.argv[5] if len(sys.argv) > 5 else DEFAULT_API_KEY
    
    print(f"[Subscriber] Connecting to {host}:{port}...")
    
    try:
        sock = create_tls_connection(host, port)
        print(f"[Subscriber] Connected!")
        
        if api_key:
            print(f"[Subscriber] Authenticating...")
            if not authenticate(sock, api_key):
                print("[Subscriber] ERROR: Authentication failed")
                sys.exit(1)
            print(f"[Subscriber] Authenticated!")
        
        print(f"[Subscriber] Subscribing to channel '{channel}'")
        response = send_command(sock, f"SUBSCRIBE {channel}")
        print(f"[Subscriber] Subscribe response: {response}")
        
        if not response.startswith("OK"):
            print(f"[Subscriber] ERROR: Failed to subscribe")
            sys.exit(1)
        
        stop_event = threading.Event()
        receiver_thread = threading.Thread(
            target=receive_messages, 
            args=(sock, stop_event)
        )
        receiver_thread.start()
        
        print(f"[Subscriber] Listening for messages (timeout: {timeout}s)...")
        print("[Subscriber] Press Ctrl+C to stop")
        
        try:
            time.sleep(timeout)
        except KeyboardInterrupt:
            print("\n[Subscriber] Interrupted by user")
        
        stop_event.set()
        receiver_thread.join(timeout=2)
        
        try:
            sock.send(b"QUIT\n")
        except:
            pass
        
        print("[Subscriber] Disconnected")
        
    except ConnectionRefusedError:
        print(f"[Subscriber] ERROR: Could not connect to {host}:{port}")
        print("[Subscriber] Make sure the server is running.")
        sys.exit(1)
    except Exception as e:
        print(f"[Subscriber] ERROR: {e}")
        sys.exit(1)
    finally:
        if 'sock' in locals():
            sock.close()


if __name__ == "__main__":
    main()
