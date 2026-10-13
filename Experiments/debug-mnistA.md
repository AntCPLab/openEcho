# MNIST full A

## Echo Online 

P0:
```bash
Time = 44.4738 seconds 
Time1 = 32.4675 seconds (786.059 MB, 1193136 rounds)
Data sent = 1676.83 MB in ~1249533 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 54.0327 seconds 
Time1 = 30.0554 seconds (786.059 MB, 1193136 rounds)
Data sent = 1676.83 MB in ~1249533 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
Instruction statistics:
        USE_INP        41
        ACTIVE         70
        GLDMS          1169
        GLDMC          2505
        RAND           2753
        PRINTCHR       3186
        PRINTINT       5251
        TIME           11039
        PRINTSTR       28649
        STOP           28955
        START          43677
        STMC           47280
        SHRCI          57707
        MULC           94121
        XORC           139965
        LDMC           148721
        CONVMODP       167810
        STMS           394940
        SHUFFLE        1122086
        SUBML          2364491
        RANDOMS        2873678
        CONDPRINTPLAIN 3334254
        CRASH          3630165
        USE_MATMUL     4316153
        SHLCI          4487790
        ADDCI          4523725
        SHRSI          4621373
        SUBSI          5449538
        DOTPRODS       7391256
        SUBINT         7950052
        MULCI          8514657
        JMPNZ          9151413
        ADDSI          11142275
        STMINT         11339539
        REQBL          12539664
        LTC            13009882
        SUBCFI         16823471
        LDMS           16921815
        CONDPRINTSTR   18001251
        LDARG          24182051
        CALL_ARG       26207718
        USE            28148030
        LDMINT         32332779
        LDMCI          33444080
        CONVCBITVEC    36921570
        UNSPLIT        37382747
        SUBMR          37772776
        NOTS           45394071
        CONVINT        46524957
        MOVSB          49077440
        LDMINTI        49939477
        MULSI          51805594
        SUBS           63911595
        MULINT         71208576
        SUBSFI         92880836
        TRUNC_PR       101063097
        LDI            104120979
        LDSI           131815414
        CONCATS        149770865
        SPLIT          160481614
        BIT            182654283
        MOVS           184631841
        ADDINT         214462248
        PICKS          233304781
        ADDM           293331237
        RUN_TAPE       313916560
        LDINT          317409610
        ADDS           331865093
        LDMSI          506114703
        REVEAL         734851372
        STMSI          1119986616
        MULS           1376676440
        XORS           2394397002
        INCINT         3416954339
        INPUTMIXED     23390726230
        JOIN_TAPE      29621392729
        MATMULSM       113268902583
        ANDS           264557192314
        CALL_TAPE      306244927856
        Total:         750248770470
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

P1:
```bash
Time = 44.7492 seconds 
Time1 = 32.474 seconds (5046.97 MB, 2578802 rounds)
Data sent = 5053.19 MB in ~2691596 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 54.3034 seconds 
Time1 = 30.0573 seconds (5046.97 MB, 2578802 rounds)
Data sent = 5053.19 MB in ~2691596 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
Instruction statistics:
        USE_INP        41
        ACTIVE         59
        PRINTINT       106
        PRINTCHR       292
        PRINTSTR       962
        GLDMS          973
        GLDMC          2419
        RAND           2745
        TIME           20126
        STOP           20418
        STMC           46785
        START          57606
        SHRCI          61072
        MULC           97664
        XORC           141725
        LDMC           152081
        CONVMODP       194941
        STMS           368468
        SHUFFLE        1147744
        SUBML          2714723
        CONDPRINTPLAIN 2733816
        CRASH          3341928
        SHLCI          4807721
        USE_MATMUL     5222503
        ADDCI          6140747
        SHRSI          6209999
        SUBSI          6699374
        SUBINT         8272009
        RANDOMS        8343350
        MULCI          9754943
        JMPNZ          10673620
        STMINT         11966453
        LTC            13514653
        ADDSI          13788135
        REQBL          15935914
        CONDPRINTSTR   17666578
        LDMS           17671342
        SUBCFI         17817054
        USE            27309653
        LDARG          28114980
        CALL_ARG       31906482
        LDMINT         35569451
        LDMCI          37799766
        CONVCBITVEC    40119193
        SUBMR          43527043
        CONVINT        47819742
        NOTS           49626301
        MOVSB          53630737
        LDMINTI        55307706
        MULSI          59960012
        SUBS           74547287
        MULINT         75828668
        SUBSFI         113049583
        LDI            115328397
        LDSI           130653057
        BIT            164245780
        CONCATS        186204130
        SPLIT          188697246
        MOVS           227573338
        ADDINT         228299382
        PICKS          265939219
        RUN_TAPE       325899744
        ADDM           331091498
        LDINT          350112576
        DOTPRODS       351041295
        ADDS           366945125
        STMSI          564734368
        LDMSI          603713554
        REVEAL         720029144
        XORS           2580764509
        INCINT         3442978841
        UNSPLIT        22285556981
        INPUTMIXED     23390598678
        JOIN_TAPE      29566575246
        TRUNC_PR       40207536089
        ANDS           52624310935
        MULS           62934440822
        MATMULSM       168227456039
        CALL_TAPE      168418282490
        Total:         579758716176
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 44.7512 seconds 
Time1 = 32.474 seconds (5046.97 MB, 2578802 rounds)
Data sent = 5053.19 MB in ~2691596 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 54.3091 seconds 
Time1 = 30.0617 seconds (5046.97 MB, 2578802 rounds)
Data sent = 5053.19 MB in ~2691596 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11783.2 MB (all parties)
Instruction statistics:
        USE_INP        61
        ACTIVE         74
        PRINTINT       93
        PRINTCHR       208
        PRINTSTR       477
        GLDMS          971
        GLDMC          2115
        RAND           2139
        STOP           21295
        TIME           21555
        STMC           42594
        START          55036
        SHRCI          59035
        MULC           88300
        XORC           141433
        LDMC           181942
        CONVMODP       193786
        STMS           371850
        SHUFFLE        1141428
        SUBML          2490035
        CONDPRINTPLAIN 2772559
        CRASH          3322946
        SHLCI          4954762
        USE_MATMUL     5454133
        ADDCI          5778572
        SHRSI          6115233
        SUBSI          6446016
        SUBINT         8044788
        RANDOMS        8391925
        MULCI          10084220
        JMPNZ          10336985
        STMINT         11755960
        ADDSI          13090533
        LTC            13554255
        REQBL          15513840
        SUBCFI         17286839
        CONDPRINTSTR   17582302
        LDMS           18011324
        USE            26570526
        LDARG          27943906
        CALL_ARG       30417364
        LDMINT         35188554
        LDMCI          37209753
        CONVCBITVEC    39220471
        SUBMR          43274860
        CONVINT        47561191
        NOTS           48074473
        MOVSB          52672616
        LDMINTI        53478376
        MULSI          58759017
        SUBS           73725448
        MULINT         75827349
        SUBSFI         107060303
        LDI            118271701
        LDSI           133332710
        BIT            172483301
        SPLIT          185920409
        CONCATS        193940707
        MOVS           199260647
        ADDINT         225796765
        PICKS          259567839
        RUN_TAPE       318894380
        ADDM           323881076
        LDINT          341740137
        ADDS           353545378
        DOTPRODS       356101978
        STMSI          554067506
        LDMSI          587427173
        REVEAL         736442389
        XORS           2584123709
        INCINT         3387371218
        UNSPLIT        21089678103
        INPUTMIXED     23390719485
        JOIN_TAPE      29565589176
        TRUNC_PR       40284349930
        ANDS           54420564861
        MULS           61747739488
        MATMULSM       168278093707
        CALL_TAPE      168748200303
        Total:         579497399902
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## RSS

P0:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 37.2404 seconds 
Time1 = 28.0038 seconds (4268.13 MB, 3398845 rounds)
Data sent = 4719.51 MB in ~3456586 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 42.313 seconds 
Time1 = 33.0843 seconds (4268.13 MB, 3398845 rounds)
Data sent = 4719.51 MB in ~3456586 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
Instruction statistics:
        USE_INP        28
        ACTIVE         45
        GLDMS          990
        RAND           1953
        GLDMC          2640
        PRINTCHR       4844
        PRINTINT       6254
        TIME           11070
        STOP           22948
        PRINTSTR       35443
        STMC           44997
        START          48707
        SHRCI          65874
        MULC           94627
        XORC           134330
        LDMC           165773
        CONVMODP       195666
        STMS           357587
        SHUFFLE        1138909
        CONDPRINTPLAIN 2365014
        CRASH          2692246
        SHLCI          4074528
        USE_MATMUL     4241579
        ADDCI          4609625
        SHRSI          5260717
        SUBML          5540104
        SUBSI          5683484
        SUBINT         6119794
        MULCI          7902664
        JMPNZ          8730859
        LTC            9194717
        RANDOMS        9249245
        STMINT         9494347
        REQBL          11866637
        CONDPRINTSTR   12811197
        SUBCFI         13656552
        LDMS           15753240
        USE            22317981
        LDARG          23229225
        CALL_ARG       24317818
        ADDSI          28167569
        LDMINT         30245020
        CONVCBITVEC    30425477
        CONVINT        32717559
        BIT            33674111
        NOTS           34797020
        MOVSB          35509284
        LDMCI          37559133
        MULSI          42164888
        LDMINTI        49832745
        SUBS           52996143
        MULINT         56853003
        SUBSFI         91115191
        LDI            91458609
        LDSI           108597788
        SUBMR          121466159
        ADDINT         155497076
        MOVS           166308976
        CONCATS        170069586
        SPLIT          184921594
        PICKS          223139448
        LDINT          245391213
        DOTPRODS       253969145
        ADDS           291205488
        RUN_TAPE       329313701
        LDMSI          555705512
        STMSI          694000337
        ADDM           1035607236
        REVEAL         1301611570
        XORS           1728423193
        INCINT         3331168763
        INPUTMIXED     8711749900
        JOIN_TAPE      32574737641
        MULS           35039528404
        TRUNC_PR       37677186116
        UNSPLIT        45263254094
        ANDS           58852098315
        MATMULSM       76108586380
        CALL_TAPE      194664722110
        Total:         500647219755
```

P1:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 37.2407 seconds 
Time1 = 28.0038 seconds (6019.22 MB, 2948605 rounds)
Data sent = 6026.9 MB in ~2950430 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 42.313 seconds 
Time1 = 33.0841 seconds (6019.22 MB, 2948605 rounds)
Data sent = 6026.9 MB in ~2950430 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         55
        PRINTINT       56
        PRINTCHR       272
        GLDMS          783
        PRINTSTR       788
        RAND           2064
        GLDMC          2362
        TIME           21412
        STOP           24142
        STMC           48986
        SHRCI          56313
        START          59184
        MULC           68247
        XORC           126716
        LDMC           163275
        CONVMODP       186913
        STMS           349197
        SHUFFLE        1142251
        CONDPRINTPLAIN 2262531
        CRASH          2315654
        SHLCI          3924969
        USE_MATMUL     4213304
        SHRSI          4949741
        ADDCI          5190086
        SUBML          5358226
        SUBSI          5806647
        SUBINT         6301896
        MULCI          8141281
        JMPNZ          8447978
        LTC            9013487
        STMINT         9231557
        RANDOMS        9468896
        CONDPRINTSTR   11606556
        REQBL          12091755
        SUBCFI         14913652
        LDMS           15627041
        USE            21037741
        LDARG          22881712
        CALL_ARG       24779476
        ADDSI          27738395
        LDMINT         29368159
        CONVCBITVEC    30589504
        CONVINT        33099631
        NOTS           34889086
        MOVSB          35625739
        LDMCI          36444644
        MULSI          42011968
        BIT            42327376
        LDMINTI        45860288
        SUBS           52001200
        MULINT         57210091
        SUBSFI         88136699
        LDI            100703894
        LDSI           104371252
        SUBMR          121201519
        ADDINT         155097617
        CONCATS        169164649
        MOVS           173468914
        SPLIT          184810877
        PICKS          222930811
        LDINT          242897657
        ADDS           274830128
        RUN_TAPE       324079856
        DOTPRODS       324736138
        INPUTMIXED     391943890
        STMSI          511236366
        LDMSI          520277507
        REVEAL         695010713
        ADDM           1024610415
        XORS           1726697398
        INCINT         3341456343
        TRUNC_PR       33255174271
        MULS           37277709207
        UNSPLIT        38368527211
        JOIN_TAPE      40909823748
        MATMULSM       74696997073
        CALL_TAPE      198284066283
        ANDS           202876909585
        Total:         637043853333
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 37.2404 seconds 
Time1 = 28.0039 seconds (4268.13 MB, 2998788 rounds)
Data sent = 4274.87 MB in ~3056369 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
```

```bash
The following benchmarks are including preprocessing (offline phase).
Time = 42.3127 seconds 
Time1 = 33.0841 seconds (4268.13 MB, 2998788 rounds)
Data sent = 4274.87 MB in ~3056369 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 15021.3 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         53
        PRINTINT       133
        PRINTCHR       205
        PRINTSTR       602
        GLDMS          1027
        RAND           1634
        GLDMC          2437
        TIME           21701
        STOP           23643
        STMC           47560
        SHRCI          54831
        START          55126
        MULC           68963
        XORC           132902
        LDMC           153445
        CONVMODP       171198
        STMS           354757
        SHUFFLE        1130278
        CONDPRINTPLAIN 2142020
        SUBML          2577188
        CRASH          2601674
        SHLCI          3774107
        USE_MATMUL     4232916
        ADDCI          5012773
        SHRSI          5076739
        SUBSI          6056375
        SUBINT         6306491
        MULCI          7692443
        JMPNZ          8296206
        STMINT         9107780
        LTC            9130480
        RANDOMS        9834922
        ADDSI          11788727
        CONDPRINTSTR   12705923
        REQBL          13084638
        SUBCFI         14091026
        LDMS           16085520
        USE            20311175
        LDARG          22819093
        CALL_ARG       24508562
        LDMINT         28976021
        CONVCBITVEC    29809140
        CONVINT        32483102
        NOTS           34551417
        MOVSB          34916648
        BIT            38094110
        LDMCI          38110380
        SUBMR          39131931
        MULSI          41024850
        LDMINTI        46063602
        SUBS           51602330
        MULINT         57024622
        SUBSFI         85854511
        LDI            92607502
        LDSI           102511140
        ADDINT         153860538
        CONCATS        167034507
        MOVS           171541306
        SPLIT          185588772
        PICKS          215187090
        LDINT          242055516
        ADDS           273871879
        ADDM           317359841
        RUN_TAPE       327209556
        DOTPRODS       436299456
        STMSI          513401605
        LDMSI          514061418
        REVEAL         720108734
        XORS           1710587885
        INCINT         3297276030
        INPUTMIXED     8713168471
        MULS           26824532202
        JOIN_TAPE      32582019758
        UNSPLIT        37436322161
        TRUNC_PR       53901555328
        ANDS           62386745951
        MATMULSM       74139979857
        CALL_TAPE      201052407784
        Total:         507254394253
```