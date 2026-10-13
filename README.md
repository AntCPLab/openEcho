# ECHO: Efficient Covertly-Secure Three-party Computation with Public Verifiability

This repository contains the experimental code for the paper “ECHO: Efficient Covertly-Secure Three-party Computation with Public Verifiability.” It provides implementations and benchmarking harnesses for the protocols evaluated in the paper across arithmetic and binary settings, optimized for different network and end-to-end objectives.

## Repository Contents (Protocols)

- **Semi-honest protocols**:
  - ABY3
  - ASTRA

- **Covert protocols**:
  - Covert MAC (Arithmetic, optimized for end-to-end)
  - Covert MAC (Arithmetic, optimized for online)
  - Covert DZKP (Arithmetic, optimized for WAN)
  - Covert ABFL (Binary, optimized for end-to-end)
  - Covert FLNW (Binary, optimized for online)
  - Covert DZKP (Binary, optimized for WAN)

- **Malicious protocols**:
  - SpdzWise Ring (Arithmetic, E2E optimal)
  - FLNW17+SPDZ2k (Arithmetic, Online optimal)
  - LEDH+24 (Arithmetic, WAN optimal)
  - FLNW17 (Binary, E2E optimal)
  - ABFL+17 (Binary, Online optimal)
  - LDHH+23 (Binary, WAN optimal)

## Build and Run

### 1) Environment setup
First, set up the MP-SPDZ environment as described in `README-MP_SPDZ.md`.

Note: when running WAN benchmarks, you can use the `traffic-control.sh` script to simulate the network condition. 

```bash
# Add a new network condition
./traffic-control.sh add <bandwidth> <latency>

# Remove a network condition
./traffic-control.sh clear
```

### 2) Build benchmark binaries
Use `make` to compile the benchmark executables:

```bash
make mul-benchmark.x and-benchmark.x and-benchmark2.x -j
```

```bash
make replicated-ring-party.x sy-rep-ring-party.x echo-prep-party.x echo-party.x covert-mac-party.x astra-prep-party.x astra-party.x -j
```

### 3) Run benchmarks

#### One command to run all NN benchmarks

Use the Python runner to orchestrate experiments and collect logs/averages with one command.

Single machine (spawns all required parties per experiment):
```bash
python3 run-NN.py                        # run all default experiments
```

Common flags:
- `--round`, `--num_gates`, `--batch_size`, `--binary_batch_size`: override global parameters
- `--host`, `--port`: network host/port passed into the benchmark binaries
- `--repeats`: number of runs per experiment (default: 3)
- `--outdir`: output directory (default: `logs/<timestamp>`)

<!-- Outputs per experiment:
- `run_XX/party_<id>.log`: stdout+stderr of each party
- `run_XX/run_meta.json`: timing and command metadata
- `experiment_summary.json`: list of runs plus per-party averaged metrics parsed from logs -->

<!-- Multi-machine usage (optional sync across machines):
1) Pick one machine to host a barrier server and optionally also run a party.
2) All machines use the same `--sync-addr host:port` and set `--num-nodes` to the number of machines participating in that experiment (e.g., 3 for 3-party).
3) On each machine, choose which parties to launch with `--party-ids`. -->
<!-- 
Example (3-party CCS on 3 machines, server also runs party 0):
```bash
# Machine A (server + party0)
python3 run-benchmarks.py ccs_mul \
  --barrier-serve --sync-addr 10.0.0.1:5000 --num-nodes 3 \
  --party-ids 0 --host 10.0.0.1 --port 12345

# Machine B (party1)
python3 run-benchmarks.py ccs_mul \
  --sync-addr 10.0.0.1:5000 --party-ids 1 \
  --host 10.0.0.1 --port 12345

# Machine C (party2)
python3 run-benchmarks.py ccs_mul \
  --sync-addr 10.0.0.1:5000 --party-ids 2 \
  --host 10.0.0.1 --port 12345
``` -->

<!-- Notes:
- The barrier server (`--barrier-serve`) only synchronizes starts/ends; it is independent from the protocol’s own networking.
- Ensure the benchmark binaries (`mul-benchmark.x`, `and-benchmark.x`, `and-benchmark2.x`) are available and executable on each machine.
- ASTRA online is a 2-party variant (parties 1 and 2 only). The one-click runner does not support ASTRA online in multi-machine mode; please run it via the separate commands below. -->


#### Separate commands to run each microbenchmarks

The commands below illustrate how to benchmark each protocol family. Replace the environment variables as needed:

- `ROUNDS`: number of protocol iterations/rounds
- `HOST`: hostname or IP of the party 0 process
- `PORT_BASE`: base port number (ports will be derived per party)
- `BATCH_SIZE`: arithmetic batch size
- `BINARY_BATCH_SIZE`: binary batch size

Run each command with party id 0/1/2 in separate terminals or machines as appropriate.

#### Semi-honest protocols
- **ABY3**
```bash
./mul-benchmark.x 0 semi -r $ROUNDS -h $HOST -pn $PORT_BASE
```

- **ASTRA**
```bash
./mul-benchmark.x 0 astra -r $ROUNDS -h $HOST -pn $PORT_BASE (on Machine 0, 1, 2)

./mul-benchmark.x 0 astra-on -r $ROUNDS -h $HOST -pn $PORT_BASE (on Machine 1, 2. Where machine 1 uses party id=0 and machine 2 uses party id=1)
```

#### Covert protocols
- **Covert MAC (Arithmetic, E2E)**
```bash
./mul-benchmark.x 0 spdz -r $ROUNDS -h $HOST -pn $PORT_BASE -S 12 -b $BATCH_SIZE
```

- **Covert MAC (Arithmetic, Online)**
```bash
./mul-benchmark.x 0 offline -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE -q 16 -lm 2
./mul-benchmark.x 0 online  -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE
```

- **Covert DZKP (Arithmetic, WAN)**
```bash
./mul-benchmark.x 0 cov-dzkp -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE -q 16 -lm 2
```
This protocol automatically uses a 12-bit CCS ring gap.

- **Covert ABFL (Binary, E2E)**
```bash
./and-benchmark.x 0 cov-trip -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

- **Covert FLNW (Binary, Online)**
```bash
./and-benchmark2.x 0 offline -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
./and-benchmark2.x 0 online  -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

- **Covert DZKP (Binary, WAN)**
```bash
./and-benchmark.x 0 cov-dzkp -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

#### Malicious protocols
- **SpdzWise Ring (Arithmetic, E2E optimal)**
```bash
./mul-benchmark.x 0 spdz -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE
```

- **FLNW17 (Arithmetic, Online optimal)**
```bash
./mul-benchmark.x 0 mal -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE
```

- **LEDH+24 (Arithmetic, WAN optimal)**
```bash
./mul-benchmark.x 0 ccs -r $ROUNDS -h $HOST -pn $PORT_BASE -b $BATCH_SIZE -lm 40 -q 16
```
This protocol automatically uses a 64-bit CCS ring gap.

- **FLNW17 (Binary, E2E optimal)**
```bash
./and-benchmark.x 0 mal -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

- **ABFL+17 (Binary, Online optimal)**
```bash
./and-benchmark.x 0 abfl -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

- **LDHH+23 (Binary, WAN optimal)**
```bash
./and-benchmark.x 0 uss -r $ROUNDS -h $HOST -pn $PORT_BASE -bb $BINARY_BATCH_SIZE
```

### Notes
- The `0` in the example commands denotes the party id (0/1/2). Launch one instance per party.
- Some protocols require parameter tuning (`-S`, `-q`, `-lm`) and small source edits as noted above in `Protocols/Ccs24MC.h`.
- Ensure networking between parties is configured (firewall, NAT traversal) according to your deployment.


