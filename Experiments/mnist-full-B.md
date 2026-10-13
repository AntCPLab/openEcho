# MNIST full-B

## Parameters

batch-size = 128, n_threads = 16, trunc_pr, split=3

## Usage

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/prep-usage.py mnist_full_B-1-128-16-debug5000-trunc_pr-split
 12062784160 bit triples
      161920 matrix multiplications (1x25 * 25x576)
      161920 matrix multiplications (1x400 * 400x64)
       81920 matrix multiplications (1x400 * 400x144)
       10240 matrix multiplications (1x8192 * 8192x25)
         640 matrix multiplications (1x73728 * 73728x25)
         480 matrix multiplications (6x128 * 128x10)
         160 matrix multiplications (7x128 * 128x10)
         640 matrix multiplications (8x10 * 10x100)
         640 matrix multiplications (8x100 * 100x10)
         640 matrix multiplications (8x100 * 100x256)
         640 matrix multiplications (8x256 * 256x100)
         640 matrix multiplications (16x128 * 128x100)
          40 matrix multiplications (62x100 * 100x10)
          40 matrix multiplications (62x256 * 256x100)
          40 matrix multiplications (63x100 * 100x10)
          40 matrix multiplications (63x256 * 256x100)
       66800 integer bits
   105455360 integer dabits
     7940000 integer inputs from player 0
      100001 integer opens
 15238887760 integer triples
```

## Commands

```bash
python3 prepare_mnist_data.py 5000 5000
./replicated-ring-party.x $ID mnist_full_B-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345
```

## ABY3 debug 5000 (LAN)
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 56.5748 seconds 
Time1 = 55.22 seconds (3734.28 MB, 3636442 rounds)
Data sent = 3800.64 MB in ~3645503 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11854.7 MB (all parties)
```

## Echo-Offline debug5000 (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-mnist_full_B-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.6408e-05
Elapsed time: 86.5334
Stopped timer 1 at 85.8865 (1621.58 MB, 2124592 rounds)
Final Check.
Final Check.
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.

=== File IO Statistics ===
Thread 0 File IO: 0.000252737 seconds
Thread 1 File IO: 0.308339 seconds
Thread 2 File IO: 0.295137 seconds
Thread 3 File IO: 0.279903 seconds
Thread 4 File IO: 0.289551 seconds
Thread 5 File IO: 0.29224 seconds
Thread 6 File IO: 0.287819 seconds
Thread 7 File IO: 0.292312 seconds
Thread 8 File IO: 0.323103 seconds
Thread 9 File IO: 0.295528 seconds
Thread 10 File IO: 0.293165 seconds
Thread 11 File IO: 0.296693 seconds
Thread 12 File IO: 0.292656 seconds
Thread 13 File IO: 0.338871 seconds
Thread 14 File IO: 0.291304 seconds
Thread 15 File IO: 0.313446 seconds
Thread 16 File IO: 0.299972 seconds
Total File IO (sum): 4.79029 seconds
Average per thread: 0.281782 seconds
Max File IO: 0.338871 seconds

=== Bytes Written Statistics ===
Total Bytes Written (sum): 219231596704 bytes (209076 MB)

=========================


The following benchmarks are including preprocessing (offline phase).
Time = 130.848 seconds 
Time1 = 85.8865 seconds (1621.58 MB, 2124592 rounds)
Data sent = 3771.85 MB in ~3446612 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 219278 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## Echo-Online (LAN)
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 41.2461 seconds 
Time1 = 26.0374 seconds (1935.89 MB, 2140800 rounds)
Data sent = 3928.21 MB in ~4150998 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 41928.1 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## ABY3 (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_B-1-128-16-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_B-1-128-16-trunc_pr-split -pn 19879 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_B-1-128-16-trunc_pr-split -pn 19879 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_B-1-128-16-trunc_pr-split -pn 19879 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8526e-05
done with epoch 0
Elapsed time: 246.716
Stopped timer 1 at 238.152 (43575.2 MB, 42464180 rounds)
test loss: 0.0724945
acc: 0.9722 (9722/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 273.144 seconds 
Time1 = 238.152 seconds (43575.2 MB, 42464180 rounds)
Data sent = 48845.2 MB in ~48873842 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 153270 MB (all parties)
```

## ABY3 debug5000 (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_B-1-128-16-debug5000-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_B-1-128-16-debug5000-trunc_pr-split -pn 13771 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_B-1-128-16-debug5000-trunc_pr-split -pn 13771 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_B-1-128-16-debug5000-trunc_pr-split -pn 13771 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8426e-05
done with epoch 0
Elapsed time: 21.6107
Stopped timer 1 at 20.3669 (3716.44 MB, 3621682 rounds)
test loss: 0.562027
acc: 0.843 (4215/5000)
The following benchmarks are including preprocessing (offline phase).
Time = 34.8215 seconds 
Time1 = 20.3669 seconds (3716.44 MB, 3621682 rounds)
Data sent = 6193.44 MB in ~6807160 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 19447.1 MB (all parties)
```