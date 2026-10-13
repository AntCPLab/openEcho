# ResNet 50

## Echo Online

P0:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 20.1458 seconds 
Time1 = 18.6387 seconds (417.558 MB, 551884 rounds)
Data sent = 829.516 MB in ~554416 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2907.67 MB (all parties)
Instruction statistics:
        USE_INP        40
        ACTIVE         57
        SUBSI          75
        LDMC           350
        STMC           788
        GLDMS          1030
        LDMS           1114
        PRINTSTR       1780
        GLDMC          2630
        PRINTCHR       2744
        PRINTREGPLAIN  19937
        ADDSI          21127
        STOP           37349
        START          62288
        USE_MATMUL     83912
        STMINT         252520
        JMPEQZ         769573
        LDARG          836275
        REQBL          1892064
        GTC            2488582
        STMS           3664377
        SUBCFI         4115086
        JMPNZ          4661135
        TRUNC_PR       4764726
        RUN_TAPE       5695762
        CALL_ARG       6647564
        USE            8517895
        UNSPLIT        10039760
        OPEN           11170391
        SUBMR          14722321
        LDMINT         16374076
        DIVINT         18027566
        MULSI          18340270
        MATMULSM       25120900
        CONCATS        31438748
        MOVS           31642950
        SUBS           70164280
        SUBSFI         75698795
        SPLIT          82384261
        LDSI           85462081
        PICKS          90479810
        ADDM           95209860
        STMSI          177648334
        MULS           187805707
        ADDS           190381954
        CONDPRINTPLAIN 271292390
        CRASH          293347521
        LDI            299121291
        INPUTPERSONAL  408240518
        LTC            518045967
        SUBINT         531711849
        XORS           838679163
        FIXINPUT       848579111
        CONDPRINTSTR   1994686017
        CONVINT        2471802778
        LDINT          2768114726
        LDMINTI        3147468101
        MULINT         4546590984
        LDMSI          7354445570
        ADDINT         7976896124
        JOIN_TAPE      18571514729
        CONV2DS        25434739058
        INCINT         38267956215
        ANDS           106397312855
        CALL_TAPE      127505066079
        Total:         351722263890
Total JMPEQZ executed: 4944
```

P1:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 20.2042 seconds 
Time1 = 18.6388 seconds (1039.07 MB, 800898 rounds)
Data sent = 1039.08 MB in ~805976 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2907.67 MB (all parties)
Instruction statistics:
        USE_INP        42
        ACTIVE         59
        SUBSI          108
        PRINTCHR       220
        PRINTSTR       260
        PRINTREGPLAIN  355
        LDMC           522
        STMC           668
        LDMS           1051
        GLDMS          1097
        GLDMC          2307
        ADDSI          21579
        STOP           41359
        OPEN           51076
        START          53008
        USE_MATMUL     81417
        FIXINPUT       231090
        STMINT         283574
        JMPEQZ         751785
        LDARG          915387
        REQBL          2425688
        GTC            3127953
        STMS           3666065
        SUBCFI         5284910
        RUN_TAPE       6348749
        JMPNZ          6398296
        USE            12809991
        CALL_ARG       12838293
        SUBMR          16421858
        DIVINT         18346309
        MULSI          18618005
        LDMINT         22249740
        MATMULSM       33417927
        MOVS           43421404
        CONCATS        58790207
        SUBS           82197604
        SUBSFI         91726409
        LDSI           92066649
        ADDM           103272914
        PICKS          103855734
        SPLIT          153404905
        STMSI          195400172
        ADDS           224629623
        CONDPRINTPLAIN 275752553
        CRASH          310993435
        LDI            325280323
        LTC            544716426
        SUBINT         551653290
        XORS           919980812
        INPUTPERSONAL  1267769001
        TRUNC_PR       1718514270
        CONDPRINTSTR   2091891399
        CONVINT        2595383691
        LDINT          2805901694
        LDMINTI        3033451959
        MULINT         4466773959
        LDMSI          6834880536
        UNSPLIT        7543208588
        ADDINT         7843528209
        JOIN_TAPE      18589552131
        ANDS           36776006636
        INCINT         38456876709
        CONV2DS        39749806914
        MULS           42179567411
        CALL_TAPE      67509757502
        Total:         287704403817
Total JMPEQZ executed: 4944
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 20.2263 seconds 
Time1 = 18.6388 seconds (1039.07 MB, 800898 rounds)
Data sent = 1039.08 MB in ~805976 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2907.67 MB (all parties)
Instruction statistics:
        USE_INP        41
        ACTIVE         55
        SUBSI          123
        PRINTREGPLAIN  242
        PRINTSTR       273
        PRINTCHR       474
        LDMC           489
        STMC           637
        LDMS           1052
        GLDMS          1052
        GLDMC          2269
        ADDSI          21442
        STOP           45847
        START          53241
        USE_MATMUL     84378
        OPEN           123750
        FIXINPUT       250733
        STMINT         270747
        JMPEQZ         800507
        LDARG          945268
        REQBL          2320642
        GTC            3499247
        STMS           3856160
        SUBCFI         4411236
        JMPNZ          6089628
        RUN_TAPE       6095759
        CALL_ARG       12638131
        USE            13177396
        SUBMR          16443075
        DIVINT         18088400
        MULSI          18470062
        LDMINT         22434594
        MATMULSM       33385873
        MOVS           41842114
        CONCATS        58257313
        SUBS           81818976
        SUBSFI         91309193
        LDSI           93319351
        ADDM           101914300
        PICKS          102686192
        SPLIT          149074837
        STMSI          195244831
        ADDS           223473198
        CONDPRINTPLAIN 282744382
        CRASH          306756874
        LDI            322676192
        LTC            541314153
        SUBINT         546708776
        XORS           951610938
        INPUTPERSONAL  1263766173
        TRUNC_PR       1790335546
        CONDPRINTSTR   2070005945
        CONVINT        2576844791
        LDINT          2794443844
        LDMINTI        3061327680
        MULINT         4503248299
        UNSPLIT        5650259979
        LDMSI          6898397591
        ADDINT         7941334605
        JOIN_TAPE      18589796657
        ANDS           37492129193
        INCINT         38123303117
        CONV2DS        39751245599
        MULS           43442264268
        CALL_TAPE      66445426077
        Total:         286648393807
Total JMPEQZ executed: 4944
```


## RSS

P0:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 15.7804 seconds 
Time1 = 14.3902 seconds (665.615 MB, 918823 rounds)
Data sent = 871.594 MB in ~921355 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2363.81 MB (all parties)
Instruction statistics:
        USE_INP        28
        ACTIVE         54
        SUBSI          116
        LDMC           463
        STMC           531
        GLDMS          830
        LDMS           1163
        PRINTSTR       2290
        GLDMC          2475
        PRINTCHR       4143
        ADDSI          17170
        PRINTREGPLAIN  17872
        STOP           50965
        USE_MATMUL     53217
        START          60058
        STMINT         218434
        JMPEQZ         690904
        LDARG          847670
        REQBL          1686953
        GTC            2359547
        STMS           3733100
        JMPNZ          4275272
        SUBCFI         4392100
        CALL_ARG       5390953
        OPEN           5837650
        RUN_TAPE       6368404
        USE            7353841
        DIVINT         12613263
        LDMINT         13377973
        MULSI          18171836
        MATMULSM       29084157
        MOVS           29510458
        CONCATS        34068887
        SUBS           52727773
        SUBMR          64174020
        SUBSFI         66567141
        PICKS          72548366
        LDSI           76852051
        SPLIT          92052499
        ADDM           134397613
        ADDS           152490660
        STMSI          180254883
        CONDPRINTPLAIN 186596386
        LDI            225447079
        CRASH          226198822
        INPUTPERSONAL  295503310
        SUBINT         363577019
        LTC            376000349
        XORS           610518754
        FIXINPUT       850004065
        CONDPRINTSTR   1460762916
        CONVINT        1775308664
        LDINT          2412671416
        TRUNC_PR       2988219025
        LDMINTI        3033188105
        MULINT         4267156129
        LDMSI          6730892214
        ADDINT         7711174740
        MULS           8323046535
        UNSPLIT        12967447392
        JOIN_TAPE      14324367755
        CONV2DS        14934728383
        ANDS           36112878178
        INCINT         39637707323
        CALL_TAPE      68778291758
        Total:         229663944100
Total JMPEQZ executed: 4944
```

P1:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 15.7857 seconds 
Time1 = 14.3959 seconds (826.604 MB, 826845 rounds)
Data sent = 826.604 MB in ~826848 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2363.81 MB (all parties)
Instruction statistics:
        USE_INP        28
        SUBSI          91
        ACTIVE         147
        PRINTCHR       224
        PRINTSTR       276
        LDMC           348
        STMC           460
        PRINTREGPLAIN  476
        GLDMS          1097
        LDMS           1202
        GLDMC          2930
        ADDSI          17259
        STOP           46629
        START          47798
        USE_MATMUL     53523
        FIXINPUT       175665
        STMINT         213045
        JMPEQZ         709362
        LDARG          839990
        REQBL          1736633
        GTC            2677150
        STMS           3812691
        JMPNZ          3996674
        SUBCFI         5106364
        CALL_ARG       5921609
        RUN_TAPE       6241982
        USE            7772560
        DIVINT         12652796
        LDMINT         13510551
        MULSI          18342415
        MATMULSM       29127124
        MOVS           30442683
        CONCATS        33279173
        SUBS           52894443
        SUBMR          65040755
        SUBSFI         66865389
        PICKS          73338730
        LDSI           75649853
        SPLIT          91015933
        INPUTPERSONAL  108761952
        ADDM           134915522
        ADDS           152315839
        STMSI          177536045
        CONDPRINTPLAIN 183444602
        CRASH          226688038
        LDI            228468023
        LTC            374153846
        SUBINT         374908436
        XORS           610234578
        OPEN           1030767985
        CONDPRINTSTR   1462346533
        CONVINT        1773057017
        LDINT          2425709056
        LDMINTI        3071282131
        TRUNC_PR       3072357247
        MULINT         4334982378
        MULS           4445382490
        LDMSI          6639388892
        ADDINT         7848205107
        UNSPLIT        12847886884
        JOIN_TAPE      14324770187
        CONV2DS        15051818546
        INCINT         39810797715
        ANDS           40048660102
        CALL_TAPE      72688807226
        Total:         234049182435
Total JMPEQZ executed: 4944
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 15.7848 seconds 
Time1 = 14.3902 seconds (665.615 MB, 892894 rounds)
Data sent = 665.615 MB in ~895426 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 2363.81 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         58
        SUBSI          123
        PRINTSTR       154
        PRINTCHR       172
        PRINTREGPLAIN  244
        LDMC           399
        STMC           439
        GLDMS          819
        LDMS           1138
        GLDMC          2383
        ADDSI          14867
        STOP           43035
        START          50445
        USE_MATMUL     60855
        OPEN           71706
        FIXINPUT       195298
        STMINT         210932
        JMPEQZ         651193
        LDARG          829349
        REQBL          1594789
        GTC            2417010
        STMS           3708479
        SUBCFI         4485035
        JMPNZ          4549430
        RUN_TAPE       6195296
        CALL_ARG       6353927
        USE            7361140
        DIVINT         12608298
        LDMINT         13429486
        SUBMR          16495069
        MULSI          18217544
        MATMULSM       28947008
        MOVS           29171809
        CONCATS        32894322
        SUBS           53378036
        SUBSFI         56925001
        LDSI           62297829
        PICKS          72546722
        ADDM           77141685
        SPLIT          90820083
        ADDS           153628731
        STMSI          173012056
        CONDPRINTPLAIN 185668919
        CRASH          223057713
        LDI            224037325
        SUBINT         372701000
        LTC            388695717
        XORS           610220467
        INPUTPERSONAL  1140082034
        CONDPRINTSTR   1465899180
        CONVINT        1786746788
        LDINT          2451155133
        LDMINTI        3073442686
        TRUNC_PR       3709756871
        MULINT         4363143433
        LDMSI          6701160924
        ADDINT         7857603555
        UNSPLIT        9823870399
        MULS           12830508586
        JOIN_TAPE      14324822967
        CONV2DS        15133922838
        ANDS           34089640478
        INCINT         39451427870
        CALL_TAPE      64320049524
        Total:         225457926830
Total JMPEQZ executed: 4944
```