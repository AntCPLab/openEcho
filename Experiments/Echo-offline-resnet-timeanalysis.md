# Echo-Offline Resnet50

## 1 thread, command line, no ccs

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# sudo perf record -F 99 -g -- ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-1-trunc_pr-split -h $P0 -pn 12345 -o "noccs"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2553e-05

Stopped timer 1 at 321.949 (64535.3 MB, 863907 rounds)
File IO: 0.191329 seconds
File IO: 1.87354 seconds
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Final Check.
[FastVector] munmap failed: Invalid argument
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 322.718 seconds 
Time1 = 321.949 seconds (64535.3 MB, 863907 rounds)
Data sent = 64535.3 MB in ~866441 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 65239.9 MB (all parties)
[ perf record: Woken up 3 times to write data ]
[ perf record: Captured and wrote 1.488 MB perf.data (18062 samples) ]
```

## 1 thread, command line, no file storage

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# sudo perf record -F 99 -g -- ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-1-trunc_pr-split -h $P0 -pn 12345 -o "nofile" -b 2097152 -q 16 -lm 2
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2795e-05
Stopped timer 1 at 305.268 (64552.3 MB, 333254 rounds)
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 305.881 seconds 
Time1 = 305.268 seconds (64552.3 MB, 333254 rounds)
Data sent = 64552.3 MB in ~333266 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 65260 MB (all parties)
```

## 32 threads, command line, no file storage

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 /usr/bin/time -v \
    ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-32-trunc_pr-split \
    -h $P0 -pn 12345 -o nofile -b 2097152 -q 16 -lm 2
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2322e-05
Stopped timer 1 at 37.3427 (64761.9 MB, 375941 rounds)
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 38.6583 seconds 
Time1 = 37.3427 seconds (64761.9 MB, 375941 rounds)
Data sent = 64761.9 MB in ~375984 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 65874.7 MB (all parties)
        Command being timed: "./echo-prep-party.x 0 echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-32-trunc_pr-split -h 172.25.88.179 -pn 12345 -o nofile -b 2097152 -q 16 -lm 2"
        User time (seconds): 601.66
        System time (seconds): 214.42
        Percent of CPU this job got: 1917%
        Elapsed (wall clock) time (h:mm:ss or m:ss): 0:42.55
        Average shared text size (kbytes): 0
        Average unshared data size (kbytes): 0
        Average stack size (kbytes): 0
        Average total size (kbytes): 0
        Maximum resident set size (kbytes): 21628988
        Average resident set size (kbytes): 0
        Major (requiring I/O) page faults: 0
        Minor (reclaiming a frame) page faults: 37678591
        Voluntary context switches: 1520349
        Involuntary context switches: 150567
        Swaps: 0
        File system inputs: 0
        File system outputs: 0
        Socket messages sent: 0
        Socket messages received: 0
        Signals delivered: 0
        Page size (bytes): 4096
        Exit status: 0
```

## 32 threads, command line, no file storage, with randbit batch size 1048576

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-32-trunc_pr-split -h $P0 -pn 12345 -b 2097152 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2164e-05
Stopped timer 1 at 44.0786 (64761.9 MB, 375941 rounds)
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 45.5296 seconds 
Time1 = 44.0786 seconds (64761.9 MB, 375941 rounds)
Data sent = 64761.9 MB in ~375984 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 65874.7 MB (all parties)
```

## 16 threads, command line, no file storage, with randbit batch size 1048576

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split -h $P0 -pn 12345 -b 2097152 -q 16 -lm 2 -o "nofile"
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.7e-05
Stopped timer 1 at 36.6678 (64757.4 MB, 352970 rounds)
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
...
Final Check.
Significant amount of unused triples of Echo share integer distorting the benchmark. For more accurate benchmarks, consider reducing the batch size with --batch-size.
The following benchmarks are including preprocessing (offline phase).
Time = 37.5228 seconds 
Time1 = 36.6678 seconds (64757.4 MB, 352970 rounds)
Data sent = 64757.4 MB in ~352997 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 65868.9 MB (all parties)
```