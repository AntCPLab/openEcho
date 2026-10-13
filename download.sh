#!/bin/bash

# Check if filename argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <executable-name>"
    exit 1
fi

# Check if P0 environment variable is set
if [ -z "$P0" ]; then
    echo "Error: P0 environment variable is not set"
    exit 1
fi

FILENAME=$1
URL="http://$P0:8888/$FILENAME"

# Step 1: Remove existing file if it exists
if [ -f "$FILENAME" ]; then
    echo "Removing existing file: $FILENAME"
    rm "$FILENAME"
fi

# Step 2: Download the file
echo "Downloading from: $URL"
if command -v wget &> /dev/null; then
    wget "$URL" -O "$FILENAME"
elif command -v curl &> /dev/null; then
    curl -o "$FILENAME" "$URL"
else
    echo "Error: Neither wget nor curl is available"
    exit 1
fi

# Check if download was successful
if [ $? -ne 0 ]; then
    echo "Error: Download failed"
    exit 1
fi

# Step 3: Make the file executable
echo "Setting executable permission"
chmod +x "$FILENAME"

echo "Successfully downloaded and set up: $FILENAME"
