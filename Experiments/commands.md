# Commands 

## Convert perf to flame

perf script | ./stackcollapse-perf.pl > out-name.folded

## Echo-offline 32 threads count perf

sudo perf record -F 299 -g --call-graph lbr -- 

sudo perf record -F 99 -g -- ./echo-prep-party.x $ID echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-32-trunc_pr-split -h $P0 -pn 12345 -o "noccs"

## Update compile_commands

bear -- make replicated-ring-party.x -j

## Perf to Count multithread data

1. `perf record -F 99 -g --call-graph dwarf ./your_program`
2. `perf script > out.perf`
3. `./stackcollapse-perf.pl --tid out.perf > out.folded`

## Download programs

```bash
 
```

```bash
mv test/172.25.88.179\:8888/Schedules/tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16.sch ./Schedules/
mv test/172.25.88.179\:8888/Bytecode/tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-* ./Bytecode/
```