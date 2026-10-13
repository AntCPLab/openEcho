# Cvt-e2e ResNet

## 16 threads, batch-size = 1048576

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# sudo perf record -F 99 -g -- ./covert-mac-party.x $ID Z52-edabit-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-split -h $P0 -pn 12345 -b 1048576 -S 12
Using statistical security parameter 12
Trying to run 52-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.2238e-05
Stopped timer 1 at 46.1436 (7625.98 MB, 5894510 rounds)
guess 249
The following benchmarks are including preprocessing (offline phase).
Time = 49.29 seconds 
Time1 = 46.1436 seconds (7625.98 MB, 5894510 rounds)
Data sent = 8038.18 MB in ~5900850 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 23702.5 MB (all parties)
[ perf record: Woken up 8 times to write data ]
[ perf record: Captured and wrote 10.070 MB perf.data (76128 samples) ]
```