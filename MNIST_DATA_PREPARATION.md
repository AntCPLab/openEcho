# MNIST Data Preparation Guide

## Overview

This guide explains how to prepare MNIST data for the mnist_full_A/B/C/D.mpc programs, particularly when using debug modes.

## The Problem

If you encounter this error:
```
Fatal error at mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split-0:91 (INPUTMIXED): 
cannot read integer from Player-Data/Input-P0-0, problem with '.521569' after 794001
```

This means the data file doesn't match the number of samples expected by the compiled program.

## Root Cause

The MPC programs read data sequentially:
1. Training labels (integers)
2. Training images (floats)
3. Test labels (integers)
4. Test images (floats)

When using debug modes like `debug1000`, the program expects only 1,000 training and 1,000 test samples. But if the data file contains 60,000 training samples, the program will read:
- First 1,000 training samples correctly
- Then expect test labels (integers)
- But encounter remaining training images (floats)
- **Error**: trying to read an integer but finding a float like `.521569`

## Solution

Use the `prepare_mnist_data.py` script with arguments matching your compilation mode:

### Basic Usage

```bash
python3 prepare_mnist_data.py [n_train] [n_test]
```

### Examples

#### Full Dataset (Default)
```bash
# For standard experiments (60,000 training, 10,000 test)
python3 prepare_mnist_data.py
# or explicitly:
python3 prepare_mnist_data.py 60000 10000

# Then compile and run:
./compile.py -R 64 mnist_full_A 1 128 16
./Scripts/ring.sh mnist_full_A-1-128-16
```

#### Debug1000 Mode
```bash
# For debug1000 experiments (1,000 training, 1,000 test)
python3 prepare_mnist_data.py 1000 1000

# Then compile and run:
./compile.py -R 64 mnist_full_C 1 128 16 debug1000 savemem trunc_pr split
./Scripts/ring.sh mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split
```

#### Debug Mode
```bash
# For debug experiments (100 training, 100 test)
python3 prepare_mnist_data.py 100 100

# Then compile and run:
./compile.py -R 64 mnist_full_A 1 128 16 debug
./Scripts/ring.sh mnist_full_A-1-128-16-debug
```

#### Debug5000 Mode
```bash
# For debug5000 experiments (5,000 training, 5,000 test)
python3 prepare_mnist_data.py 5000 5000

# Then compile and run:
./compile.py -R 64 mnist_full_B 1 128 16 debug5000
./Scripts/ring.sh mnist_full_B-1-128-16-debug5000
```

## Important Notes

⚠️ **The sample counts in data preparation MUST match the compilation parameters!**

| Compilation Flag | Training Samples | Test Samples | Command |
|-----------------|------------------|--------------|---------|
| (none) | 60,000 | 10,000 | `python3 prepare_mnist_data.py` |
| `debug` | 100 | 100 | `python3 prepare_mnist_data.py 100 100` |
| `debug1000` | 1,000 | 1,000 | `python3 prepare_mnist_data.py 1000 1000` |
| `debug5000` | 5,000 | 5,000 | `python3 prepare_mnist_data.py 5000 5000` |
| `mini` (A only) | 10 | 10 | `python3 prepare_mnist_data.py 10 10` |
| `profile` | 1,000 | 100 | `python3 prepare_mnist_data.py 1000 100` |

## Validation

The script will output a summary showing:
- Number of samples written
- Total values in the file
- File size
- Expected format

Example output:
```
Preparing MNIST data with 1000 training and 1000 test samples...
Downloading MNIST dataset...

Writing data to Player-Data/Input-P0-0...
  Writing 1000 training labels...
  Writing 1000 training images...
  Writing 1000 test labels...
  Writing 1000 test images...

✅ Data preparation complete!
Format (matching full01.py):
  Line 1: Training labels (1 line, 10000 integers)
  Lines 2-1001: Training images (1000 lines, 784 floats each)
  Line 1002: Test labels (1 line, 10000 integers)
  Lines 1003-2002: Test images (1000 lines, 784 floats each)
File size: 24.34 MB

Total values written: 1,588,000
```

## Data Format

The output file `Player-Data/Input-P0-0` contains:
1. **Training labels**: One-hot encoded as integers (0 or 1), all on one line
   - 10 values per sample (for 10 classes)
   - Example: `0 0 0 1 0 0 0 0 0 0` for label 3
2. **Training images**: Normalized pixel values (0.0 to 1.0), one sample per line
   - 784 values per sample (28×28 pixels)
3. **Test labels**: Same format as training labels
4. **Test images**: Same format as training images

## Troubleshooting

### Error: "cannot read integer ... problem with '.XXXXXX'"
- **Cause**: Data file has more samples than the program expects
- **Solution**: Re-run `prepare_mnist_data.py` with correct sample counts

### Error: "not enough inputs in Player-Data/Input-P0-0"
- **Cause**: Data file has fewer samples than the program expects
- **Solution**: Re-run `prepare_mnist_data.py` with correct sample counts

### Error: "No module named 'torchvision'"
- **Cause**: PyTorch dependencies not installed
- **Solution**: Install with `pip install torch torchvision`

## References

- Original data preparation concept: https://github.com/csiro-mlai/mnist-mpc
  - Note: This implementation has been adapted to support command-line arguments for different debug modes
- Network architecture details: See `Experiments/mnist-intro.md`
- MP-SPDZ machine learning docs: See `doc/machine-learning.rst`
