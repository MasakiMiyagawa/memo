#!/bin/bash

# Generate private key
openssl genrsa -aes128 2048 > client.key
# Generate public key
openssl req -new -key client.key > client.csr
echo " gen pubkey is done "
# Generate cert
openssl x509 -in client.csr -days 365000 -req -signkey client.key > client.crt
