# MNIST-B Performance Breakdown: ABY3 vs Echo

## Overview

This document provides a component-by-component performance comparison between ABY3 and Echo protocols on MNIST-B debug5000.

## Experiment Setup

**Program**: `mnist_full_B-1-128-16-debug5000-trunc_pr-split`
**Compile Args**: `1 128 16 debug5000 trunc_pr split no_acc`
**Threads**: 16
**Network**: LAN

## Overall Timing (from benchmark output)

| Metric | ABY3 | Echo | Gap |
|--------|------|------|-----|
| Time1 | 55.22s | 26.04s | **29.18s** |
| Rounds | 3,636,442 | 2,140,800 | 1,495,642 |
| Data Sent | 3,800 MB | 3,928 MB | -128 MB |

## How to Run with Component Timing

### Option 1: Using the convenience script

```bash
# On party 0:
./Scripts/run-mnist-timing.sh aby3 0
# On party 1:
./Scripts/run-mnist-timing.sh aby3 1
# On party 2:
./Scripts/run-mnist-timing.sh aby3 2

# Then for Echo:
./Scripts/run-mnist-timing.sh echo 0
./Scripts/run-mnist-timing.sh echo 1
./Scripts/run-mnist-timing.sh echo 2
```

### Option 2: Direct command

#### ABY3

```bash
./replicated-ring-party.x $ID echo-mnist_full_B-1-128-16-debug5000-trunc_pr-split-no_acc \
  -h $P0 -pn 12345 -o component_timing 2>&1 | tee aby3-timing.log
```

#### Echo

```bash
./echo-party.x $ID echo-mnist_full_B-1-128-16-debug5000-trunc_pr-split-no_acc \
  -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o component_timing,nofile 2>&1 | tee echo-timing.log
```

### Analysis

```bash
./Scripts/analyze-component-timings.py aby3-timing.log echo-timing.log
```

### Environment Variables

- `P0`: Hostname of party 0 (default: localhost)
- `PORT`: Base port number (default: 12345)

## Component Timing Results

### ABY3 Component Breakdown

| Component | Time (s) | % of Total |
|-----------|----------|------------|
| Arith Mul Prepare | ? | ? |
| Arith Mul Exchange | ? | ? |
| Arith Mul Finalize | ? | ? |
| **Arith Mul Total** | ? | ? |
| Dotprod Prepare | ? | ? |
| Dotprod Next | ? | ? |
| Dotprod Finalize | ? | ? |
| **Dotprod Total** | ? | ? |
| Truncation | ? | ? |
| **Protocol Total** | ? | 100% |

### Echo Component Breakdown

| Component | Time (s) | % of Total |
|-----------|----------|------------|
| Arith Mul Prepare | ? | ? |
| Arith Mul Exchange | ? | ? |
| Arith Mul Finalize | ? | ? |
| **Arith Mul Total** | ? | ? |
| Dotprod Prepare | ? | ? |
| Dotprod Next | ? | ? |
| Dotprod Finalize | ? | ? |
| **Dotprod Total** | ? | ? |
| Binary Prepare | ? | ? |
| Binary Exchange | ? | ? |
| Binary Finalize | ? | ? |
| **Binary Total** | ? | ? |
| **Protocol Total** | ? | 100% |

## Comparison Table

| Component | ABY3 (s) | Echo (s) | Delta (s) | Gap % |
|-----------|----------|----------|-----------|-------|
| Arith Mul Total | ? | ? | ? | ? |
| Dotprod Total | ? | ? | ? | ? |
| Binary Total | N/A | ? | ? | ? |
| Truncation | ? | N/A | ? | ? |
| **TOTAL** | 55.22 | 26.04 | **29.18** | **100%** |

## Root Cause Analysis

### Primary Bottleneck (>50% of gap)

**[To be filled based on results]**

### Secondary Factors (10-20% each)

**[To be filled based on results]**

### Why Microbenchmarks Didn't Predict This

The microbenchmarks showed:
- Binary AND gates: Echo 2.7x slower (0.35s vs 0.128s)
- Expected overall: Echo should be slower

But end-to-end shows Echo 47% faster because:
1. **[To be filled based on results]**
2. **[To be filled based on results]**

## Recommendations

1. **For fair comparison**: [To be filled]
2. **For Echo optimization**: [To be filled]
3. **For ABY3 optimization**: [To be filled]

## Files Modified for Instrumentation

- `Protocols/Replicated.hpp` - Added ABY3 component timing
- `Protocols/Echo.hpp` - Added Echo component timing
- `Protocols/Echo.h` - Added timing report in destructor
- `Utils/ComponentTimer.hpp` - Created timing infrastructure
- `Scripts/analyze-component-timings.py` - Analysis script

## Usage Notes

1. Enable timing with `-o component_timing` option
2. Timing report printed when protocol is destroyed (end of execution)
3. Only thread 0 prints to avoid duplicate output
4. For Echo with nofile mode: `-o component_timing,nofile`
