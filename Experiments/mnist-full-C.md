# MNIST full-C

## Parameters

batch-size = 128, n_threads = 16, trunc_pr, split=3

before running, run `python3 prepare_mnist_data.py 5000 5000` to prepare the data

## Usage

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/prep-usage.py echo-mnist_full_C-1-128-16-debug5000-trunc_pr-split
 17780038560 bit triples
      202400 matrix multiplications (1x25 * 25x576)
      506000 matrix multiplications (1x500 * 500x64)
      102400 matrix multiplications (1x1250 * 1250x144)
       40000 matrix multiplications (1x8192 * 8192x25)
         800 matrix multiplications (1x73728 * 73728x25)
         640 matrix multiplications (8x10 * 10x500)
         640 matrix multiplications (8x500 * 500x10)
         640 matrix multiplications (8x500 * 500x800)
         640 matrix multiplications (8x800 * 800x500)
         480 matrix multiplications (31x128 * 128x10)
         160 matrix multiplications (32x128 * 128x10)
         640 matrix multiplications (50x128 * 128x500)
          40 matrix multiplications (62x500 * 500x10)
          40 matrix multiplications (62x800 * 800x500)
          40 matrix multiplications (63x500 * 500x10)
          40 matrix multiplications (63x800 * 800x500)
      861000 integer bits
     7940000 integer inputs from player 0
      100001 integer opens
 55815849240 integer triples
```

## Commands

```bash
python3 prepare_mnist_data.py 5000 5000
./replicated-ring-party.x $ID mnist_full_C-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345
```

## Echo-Offline (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-mnist_full_C-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.992e-05
Elapsed time: 262.337
Stopped timer 1 at 261.643 (553555 MB, 3310040 rounds)
Final Check.
Final Check.
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).

=== Bytes Written Statistics ===
Total Bytes Written (sum): 756799287136 bytes (721740 MB)

Time = 371.184 seconds 
Time1 = 261.643 seconds (553555 MB, 3310040 rounds)
Data sent = 744488 MB in ~5300114 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 755438 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## Echo-Online (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./echo-party.x $ID echo-mnist_full_C-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.6969e-05
Elapsed time: 60.7736
Stopped timer 1 at 59.2884 (11439.8 MB, 6592144 rounds)
Final Check. Size = 40
Final Check. Size = 211144
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 91.9591 seconds 
Time1 = 59.2884 seconds (11439.8 MB, 6592144 rounds)
Data sent = 39133.1 MB in ~12476363 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 84519.5 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## ABY3 debug 5000 (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID mnist_full_C-1-128-16-debug5000-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.7262e-05
done with epoch 0
Elapsed time: 59.9275
Stopped timer 1 at 58.6355 (5944.69 MB, 7411314 rounds)
test loss: 0.365936
acc: 0.845 (4225/5000)
The following benchmarks are including preprocessing (offline phase).
Time = 83.8773 seconds 
Time1 = 58.6355 seconds (5944.69 MB, 7411314 rounds)
Data sent = 9579.78 MB in ~14087992 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 30448.1 MB (all parties)
```

## ABY3 debug 1000 (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split -pn 17348 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split -pn 17348 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split -pn 17348 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.53e-05
done with epoch 0
Elapsed time: 8.44777
Stopped timer 1 at 8.16463 (1188.56 MB, 1481906 rounds)
test loss: 1.98592
acc: 0.454 (454/1000)
The following benchmarks are including preprocessing (offline phase).
Time = 14.1657 seconds 
Time1 = 8.16463 seconds (1188.56 MB, 1481906 rounds)
Data sent = 1931.91 MB in ~2821592 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 6140.25 MB (all parties)
```

## ABY3 debug 5000 (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_C-1-128-16-debug5000-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_C-1-128-16-debug5000-trunc_pr-split -pn 13012 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_C-1-128-16-debug5000-trunc_pr-split -pn 13012 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_C-1-128-16-debug5000-trunc_pr-split -pn 13012 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8462e-05
done with epoch 0
Elapsed time: 43.7647
Stopped timer 1 at 42.486 (5944.69 MB, 7411314 rounds)
test loss: 0.412262
acc: 0.817 (4085/5000)
The following benchmarks are including preprocessing (offline phase).
Time = 71.9127 seconds 
Time1 = 42.486 seconds (5944.69 MB, 7411314 rounds)
Data sent = 9579.78 MB in ~14087992 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 30448.1 MB (all parties)
```

