# MNIST full-D

## Parameters

batch-size = 128, n_threads = 16, trunc_pr, split=3

## Usage

```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./Scripts/prep-usage.py mnist_full_D-1-128-16-2dense-trunc_pr-split
  9008440816 bit triples
      350160 matrix multiplications (1x25 * 25x196)
          16 matrix multiplications (1x100 * 100x10)
          16 matrix multiplications (1x980 * 980x100)
        2345 matrix multiplications (1x25088 * 25088x25)
        5628 matrix multiplications (6x128 * 128x10)
        1876 matrix multiplications (7x128 * 128x10)
        7504 matrix multiplications (8x10 * 10x100)
        8752 matrix multiplications (8x100 * 100x10)
        7504 matrix multiplications (8x100 * 100x980)
        8752 matrix multiplications (8x980 * 980x100)
        5628 matrix multiplications (61x128 * 128x100)
        1876 matrix multiplications (62x128 * 128x100)
      198250 integer bits
    84015316 integer dabits
    55580000 integer inputs from player 0
      200001 integer opens
 22169431466 integer triples
```

## Commands

```bash
python3 prepare_mnist_data.py
./replicated-ring-party.x $ID mnist_full_D-1-128-16-2dense-trunc_pr-split -h $P0 -pn 12345
```

## Echo-Offline (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-mnist_full_D-1-128-16-2dense-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.0668e-05
Elapsed time: 159.14
Stopped timer 1 at 157.083 (1930.05 MB, 4947279 rounds)
Final Check.
Final Check.
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.

=== File IO Statistics ===
Thread 0 File IO: 0.00322769 seconds
Thread 1 File IO: 0.462991 seconds
Thread 2 File IO: 0.467995 seconds
Thread 3 File IO: 0.480623 seconds
Thread 4 File IO: 0.475222 seconds
Thread 5 File IO: 0.462114 seconds
Thread 6 File IO: 0.563901 seconds
Thread 7 File IO: 0.450458 seconds
Thread 8 File IO: 0.589623 seconds
Thread 9 File IO: 0.464321 seconds
Thread 10 File IO: 0.466476 seconds
Thread 11 File IO: 0.456319 seconds
Thread 12 File IO: 0.479435 seconds
Thread 13 File IO: 0.494366 seconds
Thread 14 File IO: 0.464487 seconds
Thread 15 File IO: 0.500171 seconds
Thread 16 File IO: 0.482488 seconds
Total File IO (sum): 7.76422 seconds
Average per thread: 0.456719 seconds
Max File IO: 0.589623 seconds

=== Bytes Written Statistics ===
Total Bytes Written (sum): 367621732224 bytes (350591 MB)
=========================

The following benchmarks are including preprocessing (offline phase).
Time = 170.79 seconds 
Time1 = 157.083 seconds (1930.05 MB, 4947279 rounds)
Data sent = 2737.53 MB in ~5127964 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 370022 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## Echo-Online (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./echo-party.x $ID echo-mnist_full_D-1-128-16-2dense-trunc_pr-split-no_acc -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8838e-05
Elapsed time: 52.6184
Stopped timer 1 at 42.7038 (9402.4 MB, 2893192 rounds)
Final Check. Size = 469
Final Check. Size = 563730
[Online Check] o = 1266068483699537524
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Final Check. Size = 611532
[Online Check] o = 8212459173956972532
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 52.9495 seconds 
Time1 = 42.7038 seconds (9402.4 MB, 2893192 rounds)
Data sent = 9407.6 MB in ~3005826 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 21941.5 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## ABY3 (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID mnist_full_D-1-128-16-2dense-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.7652e-05
done with epoch 0
Elapsed time: 57.9716
Stopped timer 1 at 49.3349 (6553.03 MB, 2923261 rounds)
test loss: 0.198868
acc: 0.9449 (9449/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 62.6421 seconds 
Time1 = 49.3349 seconds (6553.03 MB, 2923261 rounds)
Data sent = 7512.44 MB in ~3123521 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 24064.8 MB (all parties)
```

## ABY3 (local)

```bash
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_D-1-128-16-2dense-trunc_pr-split -pn 11351 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_D-1-128-16-2dense-trunc_pr-split -pn 11351 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_D-1-128-16-2dense-trunc_pr-split -pn 11351 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8621e-05
done with epoch 0
Elapsed time: 34.1855
Stopped timer 1 at 25.4271 (6553.03 MB, 2923261 rounds)
test loss: 0.168076
acc: 0.9529 (9529/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 35.7464 seconds 
Time1 = 25.4271 seconds (6553.03 MB, 2923261 rounds)
Data sent = 7512.44 MB in ~3123521 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 24064.8 MB (all parties)
```
