# Echo-Online Resnet50

## 16 threads

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./echo-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split -h $P0 -pn 12345 -b 2097152 -q 16 -lm 2 
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.9058e-05
Stopped timer 1 at 17.0511 (417.558 MB, 1103752 rounds)
guess 6149520425398097986

=== File IO Statistics ===
Thread 0 File IO: 0.049714 seconds
Thread 1 File IO: 0.0233268 seconds
Thread 2 File IO: 0.0229081 seconds
Thread 3 File IO: 0.0227229 seconds
Thread 4 File IO: 0.0238341 seconds
Thread 5 File IO: 0.0235059 seconds
Thread 6 File IO: 0.022626 seconds
Thread 7 File IO: 0.0237005 seconds
Thread 8 File IO: 0.0219732 seconds
Thread 9 File IO: 0.0239409 seconds
Thread 10 File IO: 0.0231512 seconds
Thread 11 File IO: 0.0227597 seconds
Thread 12 File IO: 0.0231191 seconds
Thread 13 File IO: 0.0233338 seconds
Thread 14 File IO: 0.0230857 seconds
Thread 15 File IO: 0.0230054 seconds
Thread 16 File IO: 0.0234295 seconds
Total File IO (sum): 0.420137 seconds
Average per thread: 0.0247139 seconds
Max File IO: 0.049714 seconds
=========================

The following benchmarks are including preprocessing (offline phase).
Time = 18.5171 seconds 
Time1 = 17.0511 seconds (417.558 MB, 1103752 rounds)
Data sent = 829.516 MB in ~1108815 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2907.67 MB (all parties)
```