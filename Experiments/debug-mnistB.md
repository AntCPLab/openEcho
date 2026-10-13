# MNIST full B

## Analyze Results

```bash
================================================================================
INSTRUCTION STATISTICS COMPARISON: debug-mnistB.md
================================================================================

Comparing: Echo Online (P1,P2) vs RSS (P0,P1,P2)
----------------------------------------------------------------------------------------------------

Instruction          | Echo Online          | RSS                  | Difference (RSS - Echo)
----------------------------------------------------------------------------------------------------
CALL_TAPE            |      168,254,689,836 |      255,903,987,010 | +87,649,297,174 (+52.1%)
ANDS                 |      104,310,561,975 |      165,195,527,055 | +60,884,965,080 (+58.4%)
MULS                 |       89,700,274,150 |       36,684,048,745 | -53,016,225,404 (-59.1%)
UNSPLIT              |       21,200,407,008 |       34,487,637,531 | +13,287,230,524 (+62.7%)
JOIN_TAPE            |       37,139,232,378 |       50,148,968,562 | +13,009,736,184 (+35.0%)
ADDINT               |       24,385,873,859 |       17,070,207,437 |  -7,315,666,422 (-30.0%)
LDINT                |       16,410,771,622 |       11,018,761,074 |  -5,392,010,548 (-32.9%)
TRUNC_PR             |        6,612,787,563 |       10,105,098,651 |  +3,492,311,088 (+52.8%)
LDMSI                |       18,277,684,018 |       15,160,504,083 |  -3,117,179,935 (-17.1%)
MATMULSM             |        4,499,053,028 |        1,818,642,088 |  -2,680,410,940 (-59.6%)
ADDS                 |        6,428,007,780 |        4,371,218,980 |  -2,056,788,800 (-32.0%)
STMSI                |        6,612,159,576 |        5,002,020,474 |  -1,610,139,102 (-24.4%)
SUBSFI               |        4,460,705,473 |        3,265,388,840 |  -1,195,316,633 (-26.8%)
XORS                 |        3,572,138,966 |        2,419,056,089 |  -1,153,082,877 (-32.3%)
INPUTMIXED           |        1,687,909,744 |          843,713,608 |    -844,196,136 (-50.0%)
SUBS                 |        2,146,546,646 |        1,373,385,399 |    -773,161,246 (-36.0%)
PICKS                |        1,901,492,993 |        1,291,858,189 |    -609,634,804 (-32.1%)
MULINT               |          921,506,214 |          740,754,051 |    -180,752,164 (-19.6%)
BIT                  |          126,268,944 |           48,561,187 |     -77,707,758 (-61.5%)
LDMINT               |          399,309,375 |          332,990,822 |     -66,318,553 (-16.6%)
ADDM                 |           39,637,618 |           98,349,807 |     +58,712,189 (+148.1%)
SUBMR                |           26,401,364 |           73,424,447 |     +47,023,082 (+178.1%)
DIVINT               |          135,158,869 |           97,788,572 |     -37,370,297 (-27.6%)
SUBINT               |           90,704,177 |           60,362,955 |     -30,341,222 (-33.5%)
MOVS                 |          145,594,500 |          121,565,333 |     -24,029,167 (-16.5%)
REVEAL               |           64,184,442 |           84,906,569 |     +20,722,127 (+32.3%)
JMPNZ                |           58,244,062 |           43,006,703 |     -15,237,359 (-26.2%)
LTC                  |           27,608,764 |           16,687,258 |     -10,921,506 (-39.6%)
CALL_ARG             |           26,356,546 |           18,390,599 |      -7,965,947 (-30.2%)
GTC                  |           25,471,425 |           18,871,844 |      -6,599,581 (-25.9%)
----------------------------------------------------------------------------------------------------
TOTAL                |      529,600,042,274 |      628,058,920,496 |  98,458,878,222 (+18.6%)

```

## Echo Online 

P0:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 43.8281 seconds 
Time1 = 41.9486 seconds (1935.89 MB, 2140800 rounds)
Data sent = 2063.39 MB in ~2149157 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14277.2 MB (all parties)
Instruction statistics:
        USE_INP        56
        ACTIVE         65
        GLDMS          1080
        RAND           2517
        GLDMC          2543
        PRINTCHR       4358
        STMC           5615
        PRINTINT       6462
        SHRCI          9766
        MULC           10693
        TIME           13778
        CONVMODP       17235
        STOP           18251
        LDMC           19297
        XORC           28015
        PRINTSTR       28845
        START          41782
        SHUFFLE        94258
        SUBML          189604
        RANDOMS        276318
        SHRSI          368545
        SHLCI          420939
        ADDCI          429776
        CONDPRINTPLAIN 449631
        SUBSI          454815
        CRASH          486194
        DOTPRODS       673222
        USE_MATMUL     681043
        MULCI          751567
        STMS           799111
        ADDSI          951633
        JMPEQZ         971333
        LDMCI          985696
        LDMS           1366474
        CONDPRINTSTR   2532565
        CONVCBITVEC    3265546
        NOTS           3871390
        MOVSB          4231196
        STMINT         4760864
        LDARG          5154355
        CONVINT        5280392
        REQBL          6404010
        SUBCFI         6514604
        TRUNC_PR       19315459
        LDI            20249613
        GTC            21960130
        CALL_ARG       23338624
        SUBMR          23453583
        LTC            26837139
        USE            32781679
        ADDM           35172017
        UNSPLIT        39357141
        RUN_TAPE       42220156
        MULSI          46436877
        JMPNZ          52390764
        REVEAL         66770176
        SUBINT         85559215
        DIVINT         127467694
        MOVS           130101152
        LDMINTI        155422456
        LDSI           157868933
        BIT            167733137
        CONCATS        246991064
        SPLIT          358841045
        LDMINT         372860112
        MULINT         882503096
        INPUTMIXED     1688531536
        MULS           1762850276
        PICKS          1826824425
        SUBS           2089586888
        MATMULSM       3020316153
        XORS           3423145838
        SUBSFI         4251291312
        ADDS           6246376104
        STMSI          6403921092
        INCINT         8603596916
        LDINT          16045198824
        LDMSI          17061648007
        ADDINT         23882892751
        JOIN_TAPE      37113659215
        CONV2DS        57711309792
        ANDS           260801415789
        CALL_TAPE      297849341336
        Total:         752970112955
Total JMPEQZ executed: 5120
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

P1:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 44.122 seconds 
Time1 = 42.0002 seconds (6105.09 MB, 3171712 rounds)
Data sent = 6106.89 MB in ~3188426 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14277.2 MB (all parties)
Instruction statistics:
        USE_INP        40
        ACTIVE         56
        PRINTINT       95
        PRINTCHR       234
        PRINTSTR       535
        GLDMS          1050
        GLDMC          2320
        RAND           2647
        STMC           4290
        MULC           7820
        SHRCI          8557
        CONVMODP       13808
        LDMC           20736
        STOP           23312
        TIME           24715
        XORC           27450
        START          53818
        SHUFFLE        96794
        SUBML          219309
        CONDPRINTPLAIN 396891
        SHLCI          423350
        SHRSI          467571
        CRASH          473672
        ADDCI          526376
        SUBSI          622951
        STMS           763452
        USE_MATMUL     778837
        MULCI          873712
        JMPEQZ         968110
        ADDSI          1176355
        LDMS           1358922
        LDMCI          1411908
        RANDOMS        1752701
        CONDPRINTSTR   2498107
        CONVCBITVEC    3410998
        NOTS           4180652
        MOVSB          4564572
        STMINT         5033807
        CONVINT        5301847
        LDARG          5517537
        REQBL          6968485
        SUBCFI         8040214
        LDI            24043468
        GTC            26035898
        CALL_ARG       26635564
        SUBMR          26743018
        LTC            27724232
        DOTPRODS       31254475
        USE            35613593
        ADDM           40408034
        RUN_TAPE       43988913
        MULSI          52447862
        JMPNZ          55597863
        REVEAL         60456726
        SUBINT         90456291
        BIT            123880781
        DIVINT         134956925
        MOVS           150679521
        LDSI           161452098
        LDMINTI        173915342
        CONCATS        271011789
        LDMINT         398096713
        SPLIT          416548475
        MULINT         913684944
        INPUTMIXED     1686332518
        PICKS          1902647533
        SUBS           2142004079
        XORS           3578249756
        SUBSFI         4417420642
        MATMULSM       4498157125
        ADDS           6401723613
        TRUNC_PR       6522205553
        STMSI          6602030201
        INCINT         8640095429
        LDINT          16355600305
        LDMSI          18192726457
        UNSPLIT        20896210544
        ADDINT         24204827417
        JOIN_TAPE      37141837073
        CONV2DS        86404574551
        MULS           90480701599
        ANDS           105088698111
        CALL_TAPE      168693629192
        Total:         617193322836
Total JMPEQZ executed: 5120
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 44.1264 seconds 
Time1 = 41.9942 seconds (6105.09 MB, 3171712 rounds)
Data sent = 6106.89 MB in ~3188426 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 14277.2 MB (all parties)
Instruction statistics:
        USE_INP        41
        ACTIVE         55
        PRINTINT       100
        PRINTCHR       203
        PRINTSTR       851
        GLDMS          1155
        RAND           2333
        GLDMC          2543
        STMC           4960
        MULC           7864
        SHRCI          9367
        CONVMODP       15403
        LDMC           20582
        STOP           22986
        TIME           25317
        XORC           25367
        SHUFFLE        100585
        START          121657
        SUBML          208394
        CONDPRINTPLAIN 404016
        SHLCI          412361
        SHRSI          469398
        CRASH          473004
        ADDCI          535851
        SUBSI          541646
        STMS           778271
        USE_MATMUL     819380
        MULCI          839009
        JMPEQZ         980865
        ADDSI          1101983
        LDMS           1383093
        LDMCI          1405772
        CONDPRINTSTR   2471781
        RANDOMS        2683440
        CONVCBITVEC    3445490
        NOTS           4127874
        MOVSB          4453048
        STMINT         4958617
        CONVINT        5273312
        LDARG          5373266
        REQBL          6957944
        SUBCFI         7915292
        LDI            23235677
        GTC            24906952
        SUBMR          26059711
        CALL_ARG       26077528
        LTC            27493297
        DOTPRODS       30154478
        USE            35400587
        ADDM           38867202
        RUN_TAPE       43283686
        MULSI          51000039
        JMPNZ          60890260
        REVEAL         67912159
        SUBINT         90952063
        BIT            128657108
        DIVINT         135360813
        MOVS           140509479
        LDSI           160308251
        LDMINTI        171303034
        CONCATS        269518879
        LDMINT         400522037
        SPLIT          401827456
        MULINT         929327485
        INPUTMIXED     1689486971
        PICKS          1900338453
        SUBS           2151089212
        XORS           3566028176
        MATMULSM       4499948930
        SUBSFI         4503990304
        ADDS           6454291947
        STMSI          6622288950
        TRUNC_PR       6703369573
        INCINT         8673802098
        LDINT          16465942940
        LDMSI          18362641579
        UNSPLIT        21504603471
        ADDINT         24566920301
        JOIN_TAPE      37136627683
        CONV2DS        86023334018
        MULS           88919846700
        ANDS           103532425839
        CALL_TAPE      167815750479
        Total:         614434670281
Total JMPEQZ executed: 5120
This program might benefit from some protocol options.
Consider adding the following at the beginning of your code:
        program.use_edabit(True)
```

## RSS

P0:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 55.6576 seconds 
Time1 = 54.3227 seconds (3734.28 MB, 3636442 rounds)
Data sent = 3800.64 MB in ~3645503 rounds (party 0 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11854.7 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         82
        GLDMS          770
        RAND           1923
        GLDMC          2613
        STMC           3737
        PRINTCHR       4641
        MULC           5764
        PRINTINT       6074
        SHRCI          12225
        TIME           12818
        CONVMODP       16929
        LDMC           19562
        XORC           23607
        STOP           23806
        PRINTSTR       27242
        START          51546
        SHUFFLE        97748
        SHLCI          335795
        CONDPRINTPLAIN 340082
        CRASH          378925
        ADDCI          403078
        SUBML          459854
        SHRSI          495783
        SUBSI          502652
        STMS           512000
        USE_MATMUL     639395
        MULCI          639440
        JMPEQZ         973701
        LDMS           1156350
        LDMCI          1377400
        RANDOMS        1429077
        CONDPRINTSTR   1794346
        ADDSI          2409735
        CONVCBITVEC    2604824
        MOVSB          3078209
        NOTS           3100332
        CONVINT        3689984
        STMINT         4542740
        LDARG          4678047
        REQBL          5365009
        SUBCFI         6620441
        LTC            16635459
        GTC            18522598
        CALL_ARG       18984070
        LDI            19699290
        DOTPRODS       21848011
        USE            30270824
        BIT            38269712
        JMPNZ          43697956
        RUN_TAPE       44461306
        MULSI          48990785
        SUBINT         60023113
        DIVINT         96904457
        SUBMR          97239465
        MOVS           120823018
        ADDM           128807243
        REVEAL         132193446
        LDSI           150583526
        LDMINTI        172787932
        CONCATS        250094722
        LDMINT         333939158
        SPLIT          378960912
        MULINT         740812392
        INPUTMIXED     1237238311
        PICKS          1291623318
        SUBS           1391748976
        MATMULSM       1880011905
        XORS           2419704063
        SUBSFI         3409033864
        ADDS           4364792272
        STMSI          5002467558
        INCINT         9011247163
        TRUNC_PR       9549509016
        LDINT          11051234166
        LDMSI          15272367626
        ADDINT         17088193273
        CONV2DS        21031014607
        MULS           36063430712
        UNSPLIT        43301642387
        JOIN_TAPE      49762678351
        ANDS           150569569318
        CALL_TAPE      255818789444
        Total:         642528684040
Total JMPEQZ executed: 5120
```

P1:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 55.6573 seconds 
Time1 = 54.3225 seconds (4313.82 MB, 3258202 rounds)
Data sent = 4316.93 MB in ~3259147 rounds (party 1 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11854.7 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         44
        PRINTINT       165
        PRINTCHR       189
        PRINTSTR       590
        GLDMS          952
        RAND           1443
        GLDMC          2537
        STMC           4184
        MULC           5393
        SHRCI          8396
        CONVMODP       16620
        XORC           23729
        TIME           24488
        STOP           25610
        LDMC           31683
        START          50377
        SHUFFLE        96300
        SHLCI          326504
        CONDPRINTPLAIN 360258
        CRASH          376559
        SHRSI          434984
        ADDCI          437984
        SUBML          467252
        STMS           507006
        SUBSI          558421
        USE_MATMUL     640195
        MULCI          666040
        JMPEQZ         950840
        LDMS           1137400
        RANDOMS        1324109
        LDMCI          1335725
        CONDPRINTSTR   1688657
        ADDSI          2344223
        CONVCBITVEC    2656792
        NOTS           2976055
        MOVSB          3068102
        CONVINT        3642375
        STMINT         4486981
        LDARG          4930190
        REQBL          5332848
        SUBCFI         7076040
        LTC            16797074
        CALL_ARG       16997553
        GTC            19349119
        LDI            19774874
        DOTPRODS       27845380
        USE            29575687
        JMPNZ          39734858
        RUN_TAPE       43275172
        MULSI          49569978
        BIT            55070760
        INPUTMIXED     55870302
        REVEAL         60697040
        SUBINT         61350904
        SUBMR          97324839
        DIVINT         98643929
        MOVS           127126110
        ADDM           128536006
        LDSI           149678072
        LDMINTI        164296139
        CONCATS        241089510
        LDMINT         330139541
        SPLIT          381835673
        MULINT         741141900
        PICKS          1287339072
        SUBS           1361239621
        MATMULSM       1855532327
        XORS           2408673761
        SUBSFI         3411118468
        ADDS           4369159925
        STMSI          4999748655
        INCINT         8992100452
        TRUNC_PR       9176771683
        LDINT          10965595355
        LDMSI          15034293457
        ADDINT         17017539364
        CONV2DS        20819633225
        MULS           23111909043
        UNSPLIT        35917662611
        JOIN_TAPE      50923915127
        ANDS           190975964444
        CALL_TAPE      270158844588
        Total:         675790779867
Total JMPEQZ executed: 5120
```

P2:
```bash
The following benchmarks are including preprocessing (offline phase).
Time = 55.6565 seconds 
Time1 = 54.3225 seconds (3734.28 MB, 3547282 rounds)
Data sent = 3737.12 MB in ~3556263 rounds (party 2 only; rounds counted double due to multi-threading; use '-v' for more details)
Global data sent = 11854.7 MB (all parties)
Instruction statistics:
        USE_INP        29
        ACTIVE         43
        PRINTCHR       171
        PRINTINT       191
        PRINTSTR       444
        GLDMS          1015
        RAND           1416
        GLDMC          2621
        STMC           5360
        SHRCI          8677
        MULC           9711
        CONVMODP       16068
        LDMC           18406
        XORC           20528
        TIME           23558
        STOP           25159
        START          47040
        SHUFFLE        94538
        SUBML          219116
        CONDPRINTPLAIN 283258
        SHLCI          342060
        CRASH          392825
        ADDCI          437873
        SHRSI          476794
        SUBSI          492766
        STMS           509625
        USE_MATMUL     631761
        MULCI          646473
        JMPEQZ         989556
        ADDSI          1046113
        LDMS           1136990
        RANDOMS        1373717
        LDMCI          1483110
        CONDPRINTSTR   1799457
        CONVCBITVEC    2653221
        NOTS           3000677
        MOVSB          3113826
        CONVINT        3634918
        STMINT         4556539
        LDARG          4687152
        REQBL          5750351
        SUBCFI         6743204
        LTC            16629241
        GTC            18743815
        CALL_ARG       19190173
        LDI            20004922
        SUBMR          25709037
        USE            28930153
        DOTPRODS       36956676
        ADDM           37706171
        RUN_TAPE       44189383
        JMPNZ          45587294
        MULSI          49395772
        BIT            52343088
        SUBINT         59714849
        REVEAL         61829222
        DIVINT         97817329
        MOVS           116746871
        LDSI           142025668
        LDMINTI        161588074
        CONCATS        242979791
        LDMINT         334893766
        SPLIT          374630189
        MULINT         740307861
        INPUTMIXED     1238032212
        PICKS          1296612176
        SUBS           1367167600
        MATMULSM       1720382032
        XORS           2428790444
        SUBSFI         2976014189
        ADDS           4379704744
        STMSI          5003845208
        INCINT         8958490626
        LDINT          11039453701
        TRUNC_PR       11589015254
        LDMSI          15174851166
        ADDINT         17104889674
        CONV2DS        20727079273
        UNSPLIT        24243607595
        JOIN_TAPE      49760312209
        MULS           50876806480
        ANDS           154041047402
        CALL_TAPE      241734326998
        Total:         628435024685
Total JMPEQZ executed: 5120
```