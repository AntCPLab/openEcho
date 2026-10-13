# Malicious baseline for Resnet50

## Spdzwise+FLNW, 16 threads, batch-size = 100000

```bash
root@iZ2zeajscfu3pne6b8rj7gZ:~/Covert-3PC# ./sy-rep-ring-party.x $ID edabit-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-split -h $P0 -pn 12345 -b 100000 -B 3 -v
Using statistical security parameter 40
Trying to run 64-bit computation
Setup took 0.86167 seconds.
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.6033e-05
Stopped timer 1 at 145.983 (14600.2 MB, 2094150 rounds)
Compiler: ./compile.py -R 64 tf EzPC/Athos/Networks/ResNet/graphDef.bin 16 split -Y -o edabit
Main thread communication:
Broadcasting 0.636208 MB in 23630 rounds, taking 4.58571 seconds
Passing around 9483.19 MB in 72129 rounds, taking 29.9573 seconds
Receiving directly 859.506 MB in 3879 rounds, taking 2.76892 seconds
Sending directly 524.781 MB in 1349 rounds, taking 0.246814 seconds
        53384064 triples of replicated secret left
        ~13248 edaBits of size 12 left
Significant amount of unused edaBits of size 12. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
        ~11200 edaBits of size 31 left
Significant amount of unused edaBits of size 31. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
        55781056 triples of replicated secret left
Significant amount of unused triples of replicated secret distorting the benchmark. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
        192 triples of replicated secret left
Detailed costs:
    1219284280 ANDs
      19609564 integer multiplications
17 threads spent a total of 321.574 seconds (3654.04 MB, 1157022 rounds) on the online phase, 179.25 seconds (11283.5 MB, 947597 rounds) on the preprocessing/offline phase, and 2083.58 seconds idling.
Communication details (rounds and time in parallel threads counted double):
Broadcasting 15.9693 MB in 560516 rounds, taking 66.9946 seconds (at most 7.34003)
Passing around 14241.4 MB in 1458354 rounds, taking 166.714 seconds (at most 29.9651)
Receiving directly 999.332 MB in 44530 rounds, taking 6.90982 seconds (at most 2.76898)
Sending directly 664.577 MB in 42001 rounds, taking 0.809308 seconds (at most 0.246827)
CPU time = 334.703 (overall core time)
The following benchmarks are including preprocessing (offline phase).
Time = 152.194 seconds 
Time1 = 145.983 seconds (14600.2 MB, 2094150 rounds)
Data sent = 14937.9 MB in ~2105401 rounds (party 2 only; rounds counted double due to multi-threading)
Global data sent = 45148.5 MB (all parties)
Actual preprocessing cost of program:
  Type int
      20123625           Bits
      10602472         daBits
  Type bit
     210495655        Triples
  edaBits
          2048 of length 10 (strict)
      20121577 of length 12 (strict)
      20123625 of length 31 (strict)
Command line: ./sy-rep-ring-party.x 2 edabit-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-split -h 172.25.88.179 -pn 12345 -b 100000 -B 3 -v
```

## Spdzwise+FLNW, 16 threads, batch-size = 100000, online only

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./sy-rep-ring-party.x $ID edabit-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-split -b 100000 -B 3 -F -pn 12345 -h $P0
Using statistical security parameter 40
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.258e-05
REUSING DATA - ONLY FOR BENCHMARKING
REUSING DATA - ONLY FOR BENCHMARKING
REUSING DATA - ONLY FOR BENCHMARKING
REUSING DATA - ONLY FOR BENCHMARKING
REUSING DATA - ONLY FOR BENCHMARKING
REUSING DATA - ONLY FOR BENCHMARKING
Stopped timer 1 at 29.62 (1517.82 MB, 1124085 rounds)
guess 249
The following benchmarks are excluding preprocessing (offline phase).
Time = 36.1658 seconds 
Time1 = 29.62 seconds (1517.82 MB, 1124085 rounds)
Data sent = 2189.84 MB in ~1134577 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 5900.1 MB (all parties)
```

## Malicious-rep-ring, 16 threads, no edabit, batch-size = 1048576

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./malicious-rep-ring-party.x $ID tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16 -h $P0 -pn 12345 -B 3 -b 1048578 -v
Using statistical security parameter 40
Trying to run 64-bit computation
Setup took 0.397062 seconds.
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.7222e-05
Stopped timer 1 at 466.922 (313651 MB, 2141563 rounds)
guess 249
Compiler: ./compile.py -R 64 tf EzPC/Athos/Networks/ResNet/graphDef.bin 16
Main thread communication:
Broadcasting 0.002702 MB in 93 rounds, taking 0.0579726 seconds
Passing around 119.81 MB in 152 rounds, taking 0.130946 seconds
Sending directly 205.979 MB in 2529 rounds, taking 0.307465 seconds
        12 triples of malicious replicated Z2^64 left
Prep time = 0.936351 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.739 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.593 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 162.79 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.904 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 165.512 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 166.107 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 166.323 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 162.77 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.58 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 163.943 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.943 seconds.
        11168 triples of malicious replicated Z2^64 left
Prep time = 164.879 seconds.
        10466 triples of malicious replicated Z2^64 left
Prep time = 162.8 seconds.
        10466 triples of malicious replicated Z2^64 left
Prep time = 163.357 seconds.
        10466 triples of malicious replicated Z2^64 left
Prep time = 164.052 seconds.
        12770 triples of malicious replicated Z2^64 left
Prep time = 162.907 seconds.
Detailed costs:
      11114985 integer dot products
      25747369 integer inputs
    4561601948 integer multiplications
    9153929995 integer openings
17 threads spent a total of 1021.77 seconds (73450.5 MB, 900159 rounds) on the online phase, 6073.89 seconds (240407 MB, 1243939 rounds) on the preprocessing/offline phase, and 873.22 seconds idling.
Communication details (rounds and time in parallel threads counted double):
Broadcasting 25.0291 MB in 865281 rounds, taking 386.061 seconds (at most 25.1517)
Passing around 313601 MB in 1276288 rounds, taking 643.301 seconds (at most 43.2871)
Sending directly 205.979 MB in 2529 rounds, taking 0.307465 seconds
CPU time = 6564.53 (overall core time)
The following benchmarks are including preprocessing (offline phase).
Time = 469.105 seconds 
Time1 = 466.922 seconds (313651 MB, 2141563 rounds)
Data sent = 313857 MB in ~2144098 rounds (party 0 only; rounds counted double due to multi-threading)
Global data sent = 941160 MB (all parties)
Actual preprocessing cost of program:
  Type int
    4561601948        Triples
    1224714508           Bits
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
Command line: ./malicious-rep-ring-party.x 0 tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16 -h 172.25.88.179 -pn 12345 -B 3 -b 1048578 -v
```

## Spdzwise+ABFL, 16 threads, batch-size = 100000

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# sudo perf record -F 99 -g -- ./sy-rep-ring-party.x $ID edabit-tf-EzPC_Athos_Networks_ResNet_graph
Def.bin-16-split -h $P0 -pn 12345 -b 100000 -B 3 -v
Using statistical security parameter 40
Trying to run 64-bit computation
Setup took 0.806678 seconds.
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.6546e-05
Stopped timer 1 at 132.733 (13516.6 MB, 2093517 rounds)
guess 249
Compiler: ./compile.py -R 64 tf EzPC/Athos/Networks/ResNet/graphDef.bin 16 split -Y -o edabit
Main thread communication:
Broadcasting 0.65722 MB in 24443 rounds, taking 9.1168 seconds
Passing around 9353.61 MB in 72926 rounds, taking 23.6102 seconds
Receiving directly 524.781 MB in 1349 rounds, taking 1.59467 seconds
Sending directly 859.492 MB in 3876 rounds, taking 0.493247 seconds
        ~13248 edaBits of size 12 left
Significant amount of unused edaBits of size 12. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
        ~11200 edaBits of size 31 left
Significant amount of unused edaBits of size 31. This protocol has a large minimum batch size, which makes this unavoidable for small programs.
Detailed costs:
      19609564 integer multiplications
17 threads spent a total of 317.831 seconds (4936.64 MB, 1158058 rounds) on the online phase, 164.57 seconds (9251.96 MB, 945928 rounds) on the preprocessing/offline phase, and 1896.14 seconds idling.
Communication details (rounds and time in parallel threads counted double):
Broadcasting 15.9424 MB in 559844 rounds, taking 67.6131 seconds (at most 9.12001)
Passing around 13555.6 MB in 1458547 rounds, taking 144.338 seconds (at most 23.6258)
Receiving directly 664.577 MB in 42001 rounds, taking 7.83913 seconds (at most 1.59474)
Sending directly 999.263 MB in 44495 rounds, taking 2.11765 seconds (at most 0.493247)
CPU time = 450.071 (overall core time)
The following benchmarks are including preprocessing (offline phase).
Time = 141.397 seconds 
Time1 = 132.733 seconds (13516.6 MB, 2093517 rounds)
Data sent = 14586.8 MB in ~2104887 rounds (party 0 only; rounds counted double due to multi-threading)
Global data sent = 43090.9 MB (all parties)
Actual preprocessing cost of program:
  Type int
      20123625           Bits
      10602472         daBits
  edaBits
          2048 of length 10 (strict)
      20121577 of length 12 (strict)
      20123625 of length 31 (strict)
Command line: ./sy-rep-ring-party.x 0 edabit-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-split -h 172.25.88.179 -pn 12345 -b 100000 -B 3 -v
[ perf record: Woken up 3 times to write data ]
[ perf record: Captured and wrote 7.363 MB perf.data (58816 samples) ]
```