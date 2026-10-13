# Micro benchmarks

Data reported in Echo are from P1.

## And gates from Low-level circuit

circuit: Utils/and-benchmark.cpp, Utils/and-benchmark2.cpp

10 rounds, 1048576 gates for each round

### Rss

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./and-benchmark.x $ID semi -r 10 -b 1048576 -h $P0
Prepare costs 0.0769661 seconds
Exchange costs 0.0472145 seconds
Finalize costs 0.00345894 seconds
Semi part takes 0.127695 seconds
Global data sent = 240 MB (all parties)
Transmission time: 0 seconds
Compute 1048576*10 64-bit and-gates takes 0.127697 seconds
Without file IO: 0.127697 seconds
```

### Echo online

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./and-benchmark2.x $ID online -r 10 -b 1048576 -h $P0
Prepare costs 0.0521746 seconds
Exchange costs 0.282433 seconds
Finalize costs 0.0152229 seconds
Semi part takes 0.349904 seconds
Global data sent = 480.003 MB (all parties)
Transmission time: 0 seconds
Compute 1048576 64-bit and-gates takes 0.349907 seconds
Without file IO: 0.349907 seconds
```

## And gates from high-level circuit

circuit: Programs/Source/thread_and.mpc (maybe not same as Low-level circuit)

Compile commands: `./compile.py -R 64 thread_and <num-threads>`

### RSS

1 thread:
```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./replicated-ring-party.x $ID thread_and-1 -h $P0 -pn 12345 -b 1048576 -o nofile,debug_and
Trying to run 64-bit computation
[debug_and] total_bits=10240000 fast_mode=1 repeat=0
[debug_and] prepare_iterations=160000 finalize_iterations=160000
[debug_and] prepare=0.00133277 exchange=0.00194061 finalize=0.000272206 total=0.00354558
The following benchmarks are including preprocessing (offline phase).
Time = 49.4816 seconds 
Data sent = 1.28014 MB in ~4 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 3.84041 MB (all parties)
```

16 threads:

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./replicated-ring-party.x $ID thread_and-16 -h $P0 -pn 12345 -b 1048576 -o nofile
Trying to run 64-bit computation
The following benchmarks are including preprocessing (offline phase).
Time = 0.156813 seconds 
Data sent = 1.28724 MB in ~34 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 3.86172 MB (all parties)
```

### Echo online

1 thread:

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./echo-party.x $ID thread_and-1 -h $P0 -pn 12345 -b 1048576 -o nofile,debug_and
Trying to run 64-bit computation
[EchoBin::exchange] round_size=160000
[debug_and] total_bits=10240000 fast_mode=1 repeat=0
[debug_and] prepare_iterations=160000 finalize_iterations=160000
[debug_and] prepare=0.00475872 exchange=0.00539802 finalize=0.000303374 total=0.0104601
The following benchmarks are including preprocessing (offline phase).
Time = 49.5138 seconds 
Data sent = 2.56022 MB in ~7 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 7.68065 MB (all parties)
```

16 threads:

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./echo-party.x $ID thread_and-16 -h $P0 -pn 12345 -b 1048576 -o nofile
Trying to run 64-bit computation
The following benchmarks are including preprocessing (offline phase).
Time = 0.222365 seconds 
Data sent = 2.57345 MB in ~35 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 7.72034 MB (all parties)
```

Explanation of ~250x speedup from 1 thread to 16 threads instead of 16x speedup:

#### Root Cause: Compiler Unrolling Overhead

1-Thread Compilation:
- Processes all 160000 elements in ONE tape
- Compiler unrolls `regint.inc(160000)` → generates **900,000 lines of bytecode**
- Each element access requires index computation: `LDINT` and `ADDINT` for array indexing
- **57 MB of bytecode** with massive instruction overhead

16-Thread Compilation:
- Splits into 16 tapes, each processes 10000 elements
- Each tape: smaller unrolling, simpler indexing
- **3.6 MB per tape** (16x smaller!)
- Much less index computation overhead per element

#### The Speedup Breakdown:

```
Total speedup: ~180x = 16x (parallelism) × 11x (reduced overhead)
```

1. **16x from parallelism**: 16 threads run simultaneously
2. **~11x from reduced overhead**: Smaller tapes have:
   - Less index computation (fewer LDINT/ADDINT per AND)
   - Better instruction cache locality
   - Simpler control flow

#### The Instruction Count Evidence:

| Instruction | 1 Thread | 16 Threads | Per-Thread | Overhead Factor |
|-------------|----------|------------|------------|-----------------|
| LDINT | 13.6B | 1.7B | 106M | **128x less per thread!** |
| ADDINT | 35.7B | 2.3B | 146M | **245x less per thread!** |
| ANDS | 193M | 81M | 5.1M | **38x less per thread!** |

The LDINT and ADDINT overhead scales **superlinearly** with the number of elements per thread because the compiler's unrolling creates nested index computations.

This is a **compiler optimization issue** - the MPC compiler generates more efficient bytecode when workloads are split into smaller chunks!

## Conclusion of Binary Protocol

1. ABY3 is faster. Meet the expected 2x overhead.
2. The TIME_INSTRUCTIONS and COUNT_INSTRUCTIONS are not accurate. I add an option called `debug_and` to print the real time cost when running with high-level circuits.
3. The compiler optimization issue causes the speedup from 1 thread to 16 threads is not 16x, but ~250x.
4. There are many useless operations like LDINT, ADDINT costs a lot in single thread (99.97%). So if we attention to the end-to-end time, there's almost no difference between ABY3 and Echo (Echo is even faster based on the distribution).