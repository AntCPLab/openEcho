# Time Analysis

## 32 threads, time count

root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID tf-EzPC_Athos_Networks_ResNet_graphDef.bin-32-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 2.0607e-05
Stopped timer 1 at 10.7589 (650.61 MB, 790143 rounds)
guess 249
The following benchmarks are including preprocessing (offline phase).
Time = 12.4062 seconds 
Time1 = 10.7589 seconds (650.61 MB, 790143 rounds)
Data sent = 856.589 MB in ~792675 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2318.8 MB (all parties)
Instruction statistics:
        USE_INP        22
        ACTIVE         39
        SUBSI          140
        LDMC           431
        STMC           625
        GLDMS          798
        LDMS           1152
        PRINTSTR       2054
        GLDMC          2190
        PRINTCHR       4684
        ADDSI          15589
        PRINTREGPLAIN  17890
        STOP           55435
        START          61331
        OPEN           109964
        USE_MATMUL     120056
        STMINT         315100
        JMPEQZ         409397
        GTC            2245702
        REQBL          2390841
        LDARG          2455645
        LDSI           2713151
        STMS           5192699
        JMPNZ          7238032
        SUBCFI         9627089
        CALL_ARG       10480055
        DIVINT         16409987
        USE            16548462
        RUN_TAPE       18713155
        MATMULSM       23593870
        LDMINT         24275343
        MOVS           26441225
        CONCATS        73364816
        MULSI          75551381
        CONVCBITVEC    79804405
        SUBSFI         81517796
        SUBS           88575095
        SUBMR          92997308
        PICKS          101725428
        MULM           118387669
        SPLIT          185674112
        ADDS           201121507
        ADDM           215122682
        CONDPRINTPLAIN 228503698
        CRASH          261921796
        STMSI          281797675
        LDI            296153367
        INPUTPERSONAL  348222215
        LTC            468846994
        SUBINT         483653408
        XORS           712948367
        FIXINPUT       944702956
        CONDPRINTSTR   1755187567
        CONVINT        1949883760
        DABIT          5373601664
        TRUNC_PR       5456807991
        MULS           8458414274
        LDINT          9638758850
        JOIN_TAPE      10661502275
        REVEAL         12031467282
        LDMINTI        12135608956
        MULINT         14958604118
        LDMSI          22295738996
        CONV2DS        24578303894
        ADDINT         27738511637
        ANDS           50622312577
        INCINT         58889898678
        CALL_TAPE      87516398678
        Total:         359571036025

## 32 threads, flame graph

See outP0-rss-resnet50-32.folded

# 1 thread, time count

root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID tf-EzPC_Athos_Networks_ResNet_graphDef.bin-1-trunc_pr-split -h $P0 
-pn 12345
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.99e-05
Stopped timer 1 at 324.697 (650.651 MB, 778840 rounds)
guess 249
The following benchmarks are including preprocessing (offline phase).
Time = 326.161 seconds 
Time1 = 324.697 seconds (650.651 MB, 778840 rounds)
Data sent = 856.63 MB in ~781372 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2318.92 MB (all parties)
Instruction statistics:
        USE_INP        23
        ACTIVE         35
        SUBSI          259
        LDMC           560
        STMC           656
        GLDMS          656
        LDMS           1165
        PRINTSTR       1334
        PRINTCHR       2233
        GLDMC          2589
        USE_MATMUL     11938
        ADDSI          15833
        PRINTREGPLAIN  16070
        STOP           35087
        START          56689
        LDARG          75855
        STMINT         108843
        OPEN           128811
        JMPEQZ         187025
        RUN_TAPE       905349
        LDSI           1038739
        REQBL          1091651
        GTC            1816962
        JMPNZ          3343936
        CALL_ARG       3490835
        SUBCFI         4502777
        STMS           5323609
        USE            7436991
        LDMINT         9492822
        MOVS           9656612
        DIVINT         10111997
        MULSI          23603019
        MATMULSM       24288464
        CONVCBITVEC    27971140
        CONCATS        30659205
        SUBSFI         51333042
        PICKS          53063031
        SUBS           55312817
        SUBMR          66332272
        MULM           74007080
        SPLIT          79697572
        ADDS           117592123
        ADDM           149321545
        CONDPRINTPLAIN 160558865
        CRASH          161038548
        LDI            196156393
        STMSI          202600258
        LTC            310935081
        SUBINT         313331158
        INPUTPERSONAL  356260856
        XORS           401011488
        FIXINPUT       945972758
        CONDPRINTSTR   1033424594
        CONVINT        1249666784
        LDMINTI        2079480028
        LDINT          2570047249
        MULINT         3029137890
        TRUNC_PR       3352890867
        DABIT          4265212035
        MULS           4541072178
        ADDINT         5598034845
        LDMSI          6461202257
        CONV2DS        6972277022
        REVEAL         17956292999
        INCINT         35662942693
        ANDS           214861550406
        JOIN_TAPE      324579295151
        CALL_TAPE      566967228859
        Total:         1205039660513

## 1 thread, flame graph

See outP0-rss-resnet50-1.folded


# 16 threads, time count

```bash
root@iZ2zeajscfu3pne6b8rj7iZ:~/Covert-3PC# ./replicated-ring-party.x $ID tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split -h $P0 -pn 12345
Trying to run 64-bit computation
Starting timer 1 at 0 (0 MB, 0 rounds) after 1.7901e-05
Stopped timer 1 at 14.4234 (650.578 MB, 785063 rounds)
guess 249
The following benchmarks are including preprocessing (offline phase).
Time = 15.7204 seconds 
Time1 = 14.4234 seconds (650.578 MB, 785063 rounds)
Data sent = 856.557 MB in ~787595 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2318.7 MB (all parties)
```