# MNIST full-A

## Parameters

batch-size = 128, n_threads = 16, trunc_pr, split=3

## Usage

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/prep-usage.py mnist_full_A-1-128-16-trunc_pr-split
  2374153344 bit triples
        7504 matrix multiplications (8x10 * 10x128)
       15008 matrix multiplications (8x128 * 128x10)
       22512 matrix multiplications (8x128 * 128x128)
        7504 matrix multiplications (8x784 * 784x128)
        7504 matrix multiplications (49x128 * 128x128)
          16 matrix multiplications (625x128 * 128x10)
          16 matrix multiplications (625x128 * 128x128)
          16 matrix multiplications (625x784 * 784x128)
      236032 integer bits
    26384512 integer dabits
    55580000 integer inputs from player 0
      200001 integer opens
 16466262784 integer triples
```

## Command

```bash
python3 prepare_mnist_data.py 
./replicated-ring-party.x $ID mnist_full_A-1-128-16-trunc_pr-split -h $P0 -pn 12345
./echo-prep-party.x $ID echo-mnist_full_A-1-128-16-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
./flnw-sy.x $ID mnist_full_A-1-128-16-split -h $P0 -pn 12345 -b 1048576 -B 3
```

## ABY3 (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID mnist_full_A-1-128-16-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8139e-05
done with epoch 0
Elapsed time: 32.2258
Stopped timer 1 at 23.6181 (3327.78 MB, 2490292 rounds)
test loss: 0.267212
acc: 0.9303 (9303/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 32.7829 seconds 
Time1 = 23.6181 seconds (3327.78 MB, 2490292 rounds)
Data sent = 3901.19 MB in ~2575618 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 12587.6 MB (all parties)
```

## Echo-Offline (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-mnist_full_A-1-128-16-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.17e-05
Elapsed time: 133.927
Stopped timer 1 at 131.768 (253117 MB, 3544809 rounds)
Final Check.
Final Check.
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).

=== Bytes Written Statistics ===
Total Bytes Written (sum): 272850341616 bytes (260210 MB)

Time = 148.76 seconds 
Time1 = 131.768 seconds (253117 MB, 3544809 rounds)
Data sent = 272834 MB in ~3579348 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 275236 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## Echo-Online (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-party.x $ID echo-mnist_full_A-1-128-16-trunc_pr-split -h $P0 -pn 12345 -b 1048576 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.6596e-05
done with epoch 0
Elapsed time: 41.0066
Stopped timer 1 at 31.1057 (786.059 MB, 1193136 rounds)
test loss: 2.27473
acc: 0.1051 (1051/10000)
reset after reducing learning rate to 0.00498962
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Significant amount of unused bits of Echo share Z2^64 distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 41.961 seconds 
Time1 = 31.1057 seconds (786.059 MB, 1193136 rounds)
Data sent = 1755.29 MB in ~1268206 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 12195.6 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## SpdzWise + FLNW (LAN)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./flnw-sy.x $ID mnist_full_A-1-128-16-split -h $P0 -pn 12345 -b 1048576 -B 3
Using statistical security parameter 40
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.4163e-05
done with epoch 0
Elapsed time: 560.487
Stopped timer 1 at 540.122 (128727 MB, 14987177 rounds)
test loss: 0.291061
acc: 0.9264 (9264/10000)
Significant amount of unused triples of replicated secret distorting the benchmark. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
The following benchmarks are including preprocessing (offline phase).
Time = 567.155 seconds 
Time1 = 540.122 seconds (128727 MB, 14987177 rounds)
Data sent = 132338 MB in ~15161027 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 395558 MB (all parties)
```

## ABY3 dropout-late (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_A-1-128-16-dropout-late-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_A-1-128-16-dropout-late-trunc_pr-split -pn 12250 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_A-1-128-16-dropout-late-trunc_pr-split -pn 12250 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_A-1-128-16-dropout-late-trunc_pr-split -pn 12250 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.9058e-05
done with epoch 0
Elapsed time: 28.0151
Stopped timer 1 at 19.2634 (3512.2 MB, 2520308 rounds)
test loss: 0.531799
acc: 0.9142 (9142/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 28.7134 seconds 
Time1 = 19.2634 seconds (3512.2 MB, 2520308 rounds)
Data sent = 4085.61 MB in ~2605634 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 13140.9 MB (all parties)
```

## ABY3 (local)

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./Scripts/ring.sh mnist_full_A-1-128-16-trunc_pr-split
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 0 mnist_full_A-1-128-16-trunc_pr-split -pn 14731 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 1 mnist_full_A-1-128-16-trunc_pr-split -pn 14731 -h localhost
Running /root/Covert-3PC/Scripts/../replicated-ring-party.x 2 mnist_full_A-1-128-16-trunc_pr-split -pn 14731 -h localhost
Trying to run 64-bit computation
Using SGD
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.8067e-05
done with epoch 0
Elapsed time: 27.9528
Stopped timer 1 at 19.2389 (3327.78 MB, 2490292 rounds)
test loss: 0.291885
acc: 0.9266 (9266/10000)
The following benchmarks are including preprocessing (offline phase).
Time = 28.7477 seconds 
Time1 = 19.2389 seconds (3327.78 MB, 2490292 rounds)
Data sent = 3901.19 MB in ~2575618 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 12587.6 MB (all parties)
```