# ECHO is faster than RSS

## Problem Summary

The ECHO protocol is faster than the RSS protocol in the end-to-end time cost.

## Reason

Echo performs better in multi-threaded environment in computing and gates.

## Experimental data

### Original

#### Semi

Time = 53.1575 seconds 
Time1 = 51.8227 seconds (3734.28 MB, 3547282 rounds)
Data sent = 3737.12 MB in ~3556263 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11854.7 MB (all parties)

#### Echo

Time = 27.5997 seconds 
Time1 = 25.8575 seconds (6106.43 MB, 3339280 rounds)
Data sent = 6108.23 MB in ~3356042 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14279.8 MB (all parties)

### 1 thread e2e

#### Semi

Time = 217.505 seconds 
Time1 = 216.099 seconds (3734.91 MB, 3560602 rounds)
Data sent = 3744.85 MB in ~3568968 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11877.9 MB (all parties)

#### Echo

Time = 322.908 seconds 
Time1 = 317.238 seconds (24741.2 MB, 3173270 rounds)
Data sent = 24743 MB in ~3189504 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 51552.1 MB (all parties)

### No ANDS

#### Semi

Time = 12.3639 seconds 
Time1 = 11.0295 seconds (2965.33 MB, 1499482 rounds)
Data sent = 3031.48 MB in ~1508143 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 9547.2 MB (all parties)

#### Echo

Time = 20.163 seconds 
Time1 = 18.4265 seconds (4171.02 MB, 1202320 rounds)
Data sent = 4172.36 MB in ~1218682 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 8472.23 MB (all parties)

### No MULS

#### Semi

Time = 42.0904 seconds 
Time1 = 40.7627 seconds (2321.4 MB, 3000522 rounds)
Data sent = 2323.7 MB in ~3009343 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 7614.44 MB (all parties)

#### Echo

Time = 23.7742 seconds 
Time1 = 22.2568 seconds (4703.08 MB, 2637640 rounds)
Data sent = 4704.34 MB in ~2654193 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11472.1 MB (all parties)

### No CONV2DS

#### Semi

Time = 44.7534 seconds 
Time1 = 43.4293 seconds (3218.3 MB, 3535122 rounds)
Data sent = 3221.14 MB in ~3544103 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 10306.7 MB (all parties)

#### Echo

Time = 22.0132 seconds 
Time1 = 19.9778 seconds (4410.02 MB, 3316240 rounds)
Data sent = 4411.82 MB in ~3333002 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 10887 MB (all parties)
