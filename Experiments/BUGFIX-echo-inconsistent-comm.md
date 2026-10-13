# BUGFIX [SOLVED]

## Problem Summary

The Echo protocol is inconsistent in communication when running under different thread settings. Theoretically, they maybe have very little difference. But in my experiment, this gap is so large. 

I tried to comment different instructions in the task. See Processor/Instruction.hpp:1540. And I surprisingly found that if I skip all CONV2DS instructions, the communication will be consistent.

## Reason

The issue was in Echo<T>::init_dotprod() in /root/projects/Covert3PC/Covert-3PC/Protocols/Echo.hpp. The os (octetStream) buffer was not being reset between init_dotprod() calls.

## Reproduce

First of all, you have to comment the lines in Processor/Instruction.hpp:1540. And run `./Scripts/echo-online.sh echo-mnist_full_B-1-128-16-debug5000-trunc_pr-split-no_acc -b 1048576 -q 16 -lm 2 -o nofile` to get the 16 thread data, run `./Scripts/echo-online.sh echo-mnist_full_B-1-128-1-debug5000-trunc_pr-split-no_acc -b 1048576 -q 16 -lm 2 -o nofile` to get the single thread data. 

If uncomment the lines (skip the CONV2DS), and run the same command, you will find the communication will be consistent.


## Experimental data

### Original

ECHO 1 thread

Time = 414.022 seconds 
Time1 = 407.098 seconds (1938.58 MB, 2152240 rounds)
Data sent = 2066.06 MB in ~2160357 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 51552.1 MB (all parties)

ECHO 16 threads

Time = 31.6842 seconds 
Time1 = 30.1834 seconds (1935.89 MB, 2140800 rounds)
Data sent = 2063.39 MB in ~2149157 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14277.2 MB (all parties)

### No ANDS

ECHO 1 thread

Time = 378.723 seconds 
Time1 = 373.508 seconds (0.42944 MB, 2800 rounds)
Data sent = 127.47 MB in ~10742 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 45736.3 MB (all parties)

ECHO 16 threads

Time = 47.7026 seconds 
Time1 = 43.2811 seconds (0.48128 MB, 3840 rounds)
Data sent = 127.522 MB in ~11797 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 8469.55 MB (all parties)

### No MULS

1 thread
Time = 478.606 seconds 
Time1 = 473.741 seconds (1938.58 MB, 2152240 rounds)
Data sent = 2066.06 MB in ~2160357 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 48719.8 MB (all parties)

16 thread
Time = 42.2823 seconds 
Time1 = 36.6022 seconds (1935.89 MB, 2140800 rounds)
Data sent = 2063.39 MB in ~2149157 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11471.9 MB (all parties)

### NO CONV2DS

1 thread
Time = 437.093 seconds 
Time1 = 431.951 seconds (1938.58 MB, 2152240 rounds)
Data sent = 2066.06 MB in ~2160357 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 10884.1 MB (all parties)

16 threads
Time = 53.8714 seconds 
Time1 = 49.7491 seconds (1935.89 MB, 2140800 rounds)
Data sent = 2063.39 MB in ~2149157 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 10884.5 MB (all parties)

## Original data with INSTRUCTION time costs in nanoseconds (counted by the framework)

### 16 threads

Time = 40.7334 seconds 
Time1 = 38.9851 seconds (6105.09 MB, 3171824 rounds)
Data sent = 6106.89 MB in ~3188538 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14277.2 MB (all parties)
Instruction statistics:
        ACTIVE         77
        PRINTCHR       150
        PRINTINT       152
        USE_INP        186
        PRINTSTR       599
        GLDMS          998
        GLDMC          2282
        RAND           2507
        STMC           4426
        MULC           6664
        SHRCI          8514
        LDMC           19827
        XORC           22772
        STOP           25880
        TIME           27657
        CONVMODP       34111
        START          57775
        SHUFFLE        94319
        SUBML          193464
        CONDPRINTPLAIN 264947
        SHLCI          330339
        CRASH          379187
        ADDCI          404394
        SHRSI          474140
        SUBSI          475061
        STMS           502302
        MULCI          642686
        USE_MATMUL     687449
        JMPEQZ         850111
        ADDSI          1017761
        LDMS           1111492
        LDMCI          1349395
        CONDPRINTSTR   1785939
        RANDOMS        2314229
        CONVCBITVEC    2814515
        NOTS           3148278
        MOVSB          3242509
        CONVINT        3837066
        STMINT         4504784
        LDARG          4674141
        REQBL          4985196
        SUBCFI         7251401
        LTC            17078815
        GTC            19341681
        CALL_ARG       19568727
        LDI            21069675
        SUBMR          27547576
        DOTPRODS       29491329
        USE            30110170
        ADDM           38231878
        JMPNZ          40229964
        RUN_TAPE       44178486
        MULSI          48102739
        REVEAL         59880283
        SUBINT         60999924
        DIVINT         98135714
        BIT            117796866
        MOVS           127811926
        LDSI           147736754
        LDMINTI        168590908
        CONCATS        263679013
        LDMINT         344867751
        SPLIT          389998743
        MULINT         742448105
        PICKS          1321712279
        INPUTMIXED     1352790290
        SUBS           1394151416
        XORS           2534293361
        SUBSFI         3066977923
        MATMULSM       4378079670
        ADDS           4443381063
        STMSI          5016198119
        TRUNC_PR       6995982688
        INCINT         8738851392
        LDINT          11025366334
        LDMSI          14567180487
        ADDINT         17072449572
        UNSPLIT        19868307324
        JOIN_TAPE      34411909647
        CONV2DS        84167843613
        MULS           89052828757
        ANDS           103048213306
        CALL_TAPE      163618681788
        Total:         578979645738
Total JMPEQZ executed: 5120

### 1 thread

Time = 432.948 seconds 
Time1 = 430.6 seconds (24741.2 MB, 3173270 rounds)
Data sent = 24743 MB in ~3189504 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 51552.1 MB (all parties)
Instruction statistics:
        USE_INP        39
        ACTIVE         43
        PRINTINT       88
        PRINTCHR       193
        PRINTSTR       624
        GLDMS          1489
        GLDMC          2226
        RAND           2667
        MULC           8154
        STMC           8408
        SHRCI          9243
        STOP           15906
        TIME           24039
        CONVMODP       28885
        XORC           30034
        LDMC           32189
        START          50663
        USE_MATMUL     59597
        JMPEQZ         64648
        SHUFFLE        95009
        SUBML          167764
        SHLCI          222591
        ADDCI          261581
        CONDPRINTPLAIN 304118
        SHRSI          315426
        CRASH          373262
        SUBSI          383283
        LDARG          390336
        RANDOMS        452421
        MULCI          463435
        STMS           568823
        LDMCI          624243
        ADDSI          753757
        LDMS           1316208
        CONDPRINTSTR   1805056
        CONVCBITVEC    2168056
        MOVSB          2725461
        NOTS           2936931
        STMINT         3505045
        CONVINT        3608451
        RUN_TAPE       4398351
        SUBCFI         4565640
        REQBL          4883781
        BIT            5667573
        CALL_ARG       12830036
        LDI            13682625
        GTC            15216693
        LTC            16505226
        USE            17744973
        SUBMR          23096053
        ADDM           28627125
        JMPNZ          33053617
        MULSI          40117863
        DOTPRODS       44229137
        SUBINT         48016965
        MOVS           84664745
        DIVINT         85087784
        LDMINTI        123622605
        REVEAL         128880514
        LDSI           145477180
        CONCATS        208724192
        LDMINT         230514364
        SPLIT          278171007
        MULINT         616270960
        PICKS          1186321204
        SUBS           1246024569
        INPUTMIXED     2089213012
        XORS           2172130896
        SUBSFI         2719071633
        MATMULSM       3636731589
        ADDS           4070016708
        STMSI          4682022946
        TRUNC_PR       5864867168
        INCINT         7766513099
        LDINT          10215830120
        LDMSI          13237143908
        ADDINT         15600789973
        UNSPLIT        17636671107
        MULS           43087421774
        CONV2DS        83205526522
        ANDS           97183228080
        CALL_TAPE      400456577866
        JOIN_TAPE      426092723368
        Total:         1144386654943
Total JMPEQZ executed: 440