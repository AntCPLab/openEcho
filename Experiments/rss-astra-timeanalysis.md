# Time Analysis

## 16 threads, offline

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./astra-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
The following benchmarks are including preprocessing (offline phase).
Time = 15.1625 seconds 
Time1 = 7.30084 seconds (0.001024 MB, 852740 rounds)
Data sent = 0.001104 MB in ~855272 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 875.757 MB (all parties)
```

## 16 threads, online

```bash
root@iZ2zeajscfu3pne6b8rj7hZ:~/Covert-3PC# ./astra-party.x 0 echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split -h $P1 -pn 12345
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2352e-05
Stopped timer 1 at 42.4644 (714.765 MB, 1627677 rounds)
guess 249

=== File IO Statistics ===
Thread 0 File IO: 0.0322004 seconds
Thread 1 File IO: 0.0435557 seconds
Thread 2 File IO: 0.0431633 seconds
Thread 3 File IO: 0.044388 seconds
Thread 4 File IO: 0.0433859 seconds
Thread 5 File IO: 0.0435625 seconds
Thread 6 File IO: 0.0429143 seconds
Thread 7 File IO: 0.0435058 seconds
Thread 8 File IO: 0.0430261 seconds
Thread 9 File IO: 0.0446916 seconds
Thread 10 File IO: 0.0440313 seconds
Thread 11 File IO: 0.044169 seconds
Thread 12 File IO: 0.0443256 seconds
Thread 13 File IO: 0.042407 seconds
Thread 14 File IO: 0.0429259 seconds
Thread 15 File IO: 0.0443192 seconds
Thread 16 File IO: 0.0435236 seconds
Total File IO (sum): 0.730095 seconds
Average per thread: 0.0429468 seconds
Max File IO: 0.0446916 seconds
=========================

The following benchmarks are including preprocessing (offline phase).
Time = 43.8992 seconds 
Time1 = 42.4644 seconds (714.765 MB, 1627677 rounds)
Data sent = 920.744 MB in ~1632740 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 1635.51 MB (all parties)
```