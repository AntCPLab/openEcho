#!/usr/bin/env python3
"""
Prepare MNIST data for mnist_full_A/B/C/D.mpc
Usage: python3 prepare_mnist_data.py [n_train] [n_test]
  n_train: number of training samples (default: 60000)
  n_test: number of test samples (default: 10000)
  
Examples:
  python3 prepare_mnist_data.py           # Full dataset: 60000 train, 10000 test
  python3 prepare_mnist_data.py 1000 1000 # Debug1000 mode
  python3 prepare_mnist_data.py 100 100   # Debug mode
  python3 prepare_mnist_data.py 5000 5000 # Debug5000 mode

Reference: https://github.com/csiro-mlai/mnist-mpc
"""

import torchvision
import numpy as np
import os
import sys

# Parse command-line arguments
try:
    n_train = int(sys.argv[1]) if len(sys.argv) > 1 else 60000
except ValueError:
    print("Error: Invalid n_train argument. Using default 60000.")
    n_train = 60000

try:
    n_test = int(sys.argv[2]) if len(sys.argv) > 2 else 10000
except ValueError:
    print("Error: Invalid n_test argument. Using default 10000.")
    n_test = 10000

# Validate arguments
if n_train < 1 or n_train > 60000:
    print(f"Error: n_train must be between 1 and 60000. Got {n_train}")
    sys.exit(1)
if n_test < 1 or n_test > 10000:
    print(f"Error: n_test must be between 1 and 10000. Got {n_test}")
    sys.exit(1)

print(f"Preparing MNIST data with {n_train} training and {n_test} test samples...")
print("Downloading MNIST dataset...")
train_ds = torchvision.datasets.MNIST(root='/tmp', train=True, download=True)
test_ds = torchvision.datasets.MNIST(root='/tmp', train=False, download=True)

# Training data: take first n_train samples
train_images = train_ds.data.numpy()[:n_train].reshape(n_train, -1) / 255.0  # Normalize to [0,1]
train_labels = train_ds.targets.numpy()[:n_train]

# Test data: take first n_test samples
test_images = test_ds.data.numpy()[:n_test].reshape(n_test, -1) / 255.0
test_labels = test_ds.targets.numpy()[:n_test]

# Write to Player-Data/Input-P0-0
# Format matches csiro-mlai/mnist-mpc:
#   - Labels: one-hot as INTEGERS (0 or 1), space-separated
#   - Images: floats with 6 decimal places, space-separated
output_file = 'Player-Data/Input-P0-0'
print(f"\nWriting data to {output_file}...")

# Create output directory if it doesn't exist
os.makedirs('Player-Data', exist_ok=True)

def write_labels(f, labels):
    """Write labels as one-hot integers (all samples in one line, space-separated)"""
    for label in labels:
        # Write one-hot: "0 0 0 1 0 0 0 0 0 0 " for label=3
        for i in range(10):
            f.write(str(int(i == label)))
            f.write(' ')

def write_images(f, images):
    """Write images as floats (one sample per line, matching full01.py format)"""
    for img in images:
        for x in img:
            f.write(f'{x:.6f} ')
        f.write('\n')  # Each sample on its own line

with open(output_file, 'w') as f:
    # Format matching full01.py:
    # - Labels: all samples in one line (space-separated)
    # - Images: one sample per line (space-separated pixels)
    
    # 1. Training labels: 1 line with (n_train × 10) integers
    print(f"  Writing {n_train} training labels...")
    write_labels(f, train_labels)
    f.write('\n')
    
    # 2. Training images: n_train lines, each with 784 floats
    print(f"  Writing {n_train} training images...")
    write_images(f, train_images)
    # write_images already writes '\n' after each sample
    
    # 3. Test labels: 1 line with (n_test × 10) integers
    print(f"  Writing {n_test} test labels...")
    write_labels(f, test_labels)
    f.write('\n')
    
    # 4. Test images: n_test lines, each with 784 floats
    print(f"  Writing {n_test} test images...")
    write_images(f, test_images)
    # write_images already writes '\n' after each sample

print("\n✅ Data preparation complete!")
print(f"Format (matching full01.py):")
print(f"  Line 1: Training labels (1 line, {n_train * 10} integers)")
print(f"  Lines 2-{n_train + 1}: Training images ({n_train} lines, 784 floats each)")
print(f"  Line {n_train + 2}: Test labels (1 line, {n_test * 10} integers)")
print(f"  Lines {n_train + 3}-{n_train + n_test + 2}: Test images ({n_test} lines, 784 floats each)")
file_size_mb = os.path.getsize(output_file) / 1024 / 1024
print(f"File size: {file_size_mb:.2f} MB")
print(f"\nTotal values written: {(n_train + n_test) * (10 + 784):,}")

# Provide context-appropriate examples
print("\nYou can now run:")
if n_train == 1000 and n_test == 1000:
    print(f"  ./compile.py -R 64 mnist_full_C 1 128 16 debug1000 savemem trunc_pr split")
    print(f"  ./Scripts/ring.sh mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split")
elif n_train == 100 and n_test == 100:
    print(f"  ./compile.py -R 64 mnist_full_A 1 128 16 debug")
    print(f"  ./Scripts/ring.sh mnist_full_A-1-128-16-debug")
elif n_train == 5000 and n_test == 5000:
    print(f"  ./compile.py -R 64 mnist_full_B 1 128 16 debug5000")
    print(f"  ./Scripts/ring.sh mnist_full_B-1-128-16-debug5000")
else:
    print(f"  ./compile.py -R 64 mnist_full_A 1 128 16")
    print(f"  ./Scripts/ring.sh mnist_full_A-1-128-16")
