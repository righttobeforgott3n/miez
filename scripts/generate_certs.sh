#!/bin/bash

# Generate self-signed TLS certificates for testing
# Usage: ./generate_certs.sh [output_dir]

OUTPUT_DIR="${1:-./certs}"

mkdir -p "$OUTPUT_DIR"

openssl genrsa -out "$OUTPUT_DIR/server.key" 2048

openssl req -new -x509 \
    -key "$OUTPUT_DIR/server.key" \
    -out "$OUTPUT_DIR/server.crt" \
    -days 365 \
    -subj "/C=US/ST=Test/L=Test/O=Test/OU=Test/CN=localhost"

cat "$OUTPUT_DIR/server.crt" "$OUTPUT_DIR/server.key" > "$OUTPUT_DIR/server.pem"

chmod 600 "$OUTPUT_DIR/server.key"
chmod 644 "$OUTPUT_DIR/server.crt"
chmod 600 "$OUTPUT_DIR/server.pem"

echo "Certificates generated in $OUTPUT_DIR:"
echo "  - server.key  (private key)"
echo "  - server.crt  (certificate)"
echo "  - server.pem  (combined)"
