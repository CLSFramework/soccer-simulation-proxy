#!/bin/sh

# sudo apt-get update && sudo apt-get install -y protobuf-compiler wget

# # Download the appropriate precompiled binary for Linux
# wget https://github.com/pseudomuto/protoc-gen-doc/releases/download/v1.5.1/protoc-gen-doc_1.5.1_linux_amd64.tar.gz -O protoc-gen-doc.tar.gz

# # Extract the binary from the tarball
# tar -xvf protoc-gen-doc.tar.gz
                
# # Ensure it's an executable binary
# file protoc-gen-doc

# # Make it executable
# chmod +x protoc-gen-doc

# # Move the binary to /usr/local/bin
# sudo mv protoc-gen-doc /usr/local/bin/

protoc --doc_out=./ --doc_opt=markdown,test.md ./grpc/service.proto

VERSION=$(grep -oP '(?<=version\s)[0-9]+\.[0-9]+' ./grpc/service.proto)
echo "VERSION=$VERSION"

sed -i '3a\\n## Version: '"$VERSION"'\n' ./test.md

sed -i 's/&gt;/>/g' ./test.md
sed -i 's/&lt;/</g' ./test.md