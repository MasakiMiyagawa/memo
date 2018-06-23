#!/bin/bash

# Generate private key
openssl genrsa -aes128 2048 > server.key
# Generate public key
openssl req -new -key server.key > server.csr
# Generate cert
openssl x509 -in server.csr -days 365000 -req -signkey server.key > server.crt
