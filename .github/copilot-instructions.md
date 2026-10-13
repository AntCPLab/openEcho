# Copilot Instructions for Covert-3PC (ECHO Protocol)

## Repository Overview

This is **ECHO: Efficient Covertly-Secure Three-party Computation with Public Verifiability**, a research implementation built on top of MP-SPDZ. The repository provides implementations of semi-honest, covert, and malicious MPC protocols for both arithmetic and binary circuits.

**Key Facts:**
- **Size:** Large-scale research codebase with 200+ protocol implementations, 500+ source files
- **Languages:** C++17, Python 3, MPC domain-specific language (.mpc files)
- **Base Framework:** MP-SPDZ (Multi-Protocol SPDZ)
- **Target:** 3-party protocols optimized for LAN/WAN networks
- **Build System:** GNU Make + CMake (for dependencies)
- **Compiler:** clang++ (default, configurable in CONFIG.mine)

## Critical Build Instructions

### Prerequisites

**ALWAYS install these dependencies first:**
```bash
# Ubuntu/Debian - Install system packages including xxhash and blake3 for ECHO
sudo apt-get install automake build-essential clang cmake git \
  libboost-dev libboost-filesystem-dev libboost-iostreams-dev \
  libboost-thread-dev libgmp-dev libntl-dev libsodium-dev \
  libssl-dev libtool python3 libxxhash-dev libblake3-dev
```

**Note:** libxxhash and libblake3 are REQUIRED for ECHO protocol implementation.

### Build Process (Sequential - Do NOT Skip Steps)

**Step 1: One-Time Setup (10-15 minutes)**
```bash
# Use make tldr for initial environment setup
make tldr
```
**Important:** This is a ONE-TIME setup that runs `make setup` internally (cloning submodules libOTe, SimplestOT_C, building dependencies) plus builds `mascot-party.x` and creates Player-Data directory. You do NOT need to run this before each compilation.

**Step 2: Configure Build (ALWAYS do this before first compilation)**
```bash
# Create CONFIG.mine if it doesn't exist - it's gitignored
touch CONFIG.mine

# REQUIRED: Add compiler flags for VLA extension warnings
echo "CXXFLAGS += -Wno-vla-extension -Wno-error=vla-extension" >> CONFIG.mine
echo "CFLAGS += -Wno-vla-extension -Wno-error=vla-extension" >> CONFIG.mine
echo "CXX = clang++" >> CONFIG.mine

# Optional: Add for faster compilation
echo "MY_CFLAGS += -DFEWER_PRIMES -DFEWER_RINGS" >> CONFIG.mine
```

**Step 3: Setup TensorFlow Inference (ALWAYS required)**
See README-MP_SPDZ.md section "### TensorFlow inference" for details. Install required packages:
```bash
pip3 install numpy scipy pillow>=9.1 tensorflow
```

**Step 4: Build Protocol Binaries**
```bash
# Build benchmark binaries (always use -j for parallel compilation)
make -j mul-benchmark.x and-benchmark.x and-benchmark2.x

# Build required protocol parties for ECHO/Covert research
make -j replicated-ring-party.x sy-rep-ring-party.x echo-prep-party.x echo-party.x covert-mac-party.x
```

**Build time:** 15-30 minutes on first build. 

**Source files:**
- `Utils/mul-benchmark.cpp` → `mul-benchmark.x` (micro-benchmarks, arithmetic protocols)
- `Utils/and-benchmark.cpp` → `and-benchmark.x` (micro-benchmarks, binary protocols, variant 1)
- `Utils/and-benchmark2.cpp` → `and-benchmark2.x` (micro-benchmarks, binary protocols, variant 2)
- `Machines/echo-party.cpp`, `Machines/echo-prep-party.cpp` → ECHO protocol, for full benchmarks like PPML
- `Machines/covert-mac-party.cpp` → Covert MAC protocol, for full benchmarks like PPML
- `Machines/replicated-ring-party.cpp` → Semi-honest baseline.
- `Machines/sy-rep-ring-party.cpp` → Malicious baseline.

**Common Build Errors & Solutions:**
1. **"undefined reference to boost"** → Run `make -j boost` first, then retry
2. **"libOTe not found"** → Run `make tldr -j` again (not make setup)
3. **VLA extension warnings** → Ensure CONFIG.mine has the required CXXFLAGS/CFLAGS
4. **Compilation too slow** → Add `-DFEWER_PRIMES -DFEWER_RINGS` to CONFIG.mine and always use `-j` flag

## Running Benchmarks

### Quick Test (Single Machine, 3 Parties)

```bash
./compile.py -R 64 mul-test
./Scripts/ring.sh mul-test
./Scripts/sy-rep-ring.sh mul-test

# echo includes prep and online phase. 
./Scripts/echo-prep.sh mul-test
./Scripts/echo-online.sh mul-test


```

**Outputs:** Results go to `logs/<timestamp>/` with:
- `run_XX/party_<id>.log` - stdout/stderr per party
- `run_XX/run_meta.json` - timing metadata
- `experiment_summary.json` - aggregated results

### Manual Protocol Execution

**Common Flags:**
- `-h` : hostname of party 0
- `-pn` : base port number
- `-b` : arithmetic batch size, always 1048576
- `-q` : statistical security parameter, always 16
- `-lm` : leakage multiplier for covert protocols, always 2

## Project Architecture

### Directory Structure

```
Covert-3PC/
├── Protocols/          # Protocol implementations (.h and .hpp files)
│   ├── AstraShare.h   # ASTRA protocol shares
│   ├── EchoShare.h    # ECHO protocol (main contribution)
│   ├── CovertMC.h     # Covert MAC checking
│   └── ...            # SpdzWise, Shamir, Replicated protocols
├── Machines/          # Protocol virtual machines (C++ implementations)
│   ├── astra-party.cpp
│   ├── echo-party.cpp
│   ├── covert-mac-party.cpp
│   └── *-party.cpp    # Each protocol has a corresponding party machine
├── Utils/             # Benchmark executables & utilities
│   ├── mul-benchmark.cpp
│   ├── and-benchmark.cpp
│   ├── and-benchmark2.cpp
│   └── *.cpp          # Other utilities (Fake-Offline, gen_input, etc.)
├── Programs/Source/   # .mpc source files (MPC high-level language)
│   ├── mul-benchmark.mpc  # Reference implementation
│   ├── and-benchmark.mpc  # Reference implementation
│   ├── tutorial.mpc
│   └── *.mpc          # Example programs
├── Compiler/          # Python MPC compiler infrastructure
├── Math/, Tools/, Networking/  # Core libraries (compiled into libSPDZ.so)
├── GC/, OT/, FHE/     # Garbled circuits, oblivious transfer, FHE
├── Scripts/           # Protocol runners (shell and Python scripts)
│   ├── echo.sh, astra.sh, mascot.sh  # Run compiled .mpc programs
│   ├── test_tutorial.sh  # Validation tests
│   └── test_ecdsa.sh     # Validation tests
├── deps/              # External dependencies (libOTe, SimplestOT_C)
├── local/             # Installed dependencies (lib/, include/, bin/)
├── Makefile           # Main build system (1000+ lines)
├── CONFIG, CONFIG.mine # Build configuration
├── compile.py         # MPC compiler driver (.mpc → bytecode)
├── run-benchmarks.py  # Multi-party benchmark orchestrator
├── README.md          # Documentation
└── README-MP_SPDZ.md  # MP-SPDZ environment setup guide
```

### Key Configuration Files

- **CONFIG** - Default compiler flags, architecture settings, dependency paths
- **CONFIG.mine** - User overrides (gitignored, create with `touch CONFIG.mine`)
- **.gitignore** - Excludes: `*.x`, `*.o`, `*.so`, `local/`, `Player-Data/`, `logs/`

### Build Artifacts (Gitignored)

- `*.x` - Executables (benchmarks, party machines)
- `*.o`, `*.d` - Object files and dependencies
- `*.so`, `*.a` - Shared/static libraries (libSPDZ.so, libFHE.so)
- `local/` - Installed dependencies (boost, libOTe, cryptoTools)
- `Programs/Bytecode/*.bc`, `Programs/Schedules/*.sch` - Compiled .mpc programs

## Validation & CI

### CI Pipeline (azure-pipelines.yml)

**Platform:** Ubuntu 24.04  
**Steps:**
1. Install dependencies (boost, sodium, ntl, gmp, xxhash, blake3)
2. Run `make setup` (equivalent to running setup portion of `make tldr`)
3. Configure: Add VLA flags, `USE_NTL=1`, `FEWER_PRIMES`, `FEWER_RINGS`, `COMP_SEC=10` to CONFIG.mine
4. Build with `-j`: `make -j replicated-{ring,field}-party.x shamir mascot-party.x echo-* covert-mac-party.x`
5. Setup SSL: `Scripts/setup-ssl.sh 4`
6. Test: `skip_binary=1 slim=1 Scripts/test_tutorial.sh -X`

**Build time in CI:** ~15-20 minutes

### Local Validation

```bash
# Run tutorial test (standard MP-SPDZ validation)
Scripts/test_tutorial.sh -X

# Test specific protocols
Scripts/echo.sh tutorial
Scripts/astra.sh tutorial
```

## Common Issues & Workarounds

1. **"make: *** [mul-benchmark.x] Error 1"**
   - Ensure `make tldr` completed successfully (one-time setup)
   - Check `local/lib/` exists and contains libOTe, cryptoTools
   - Try: `make clean && make -j mul-benchmark.x`

2. **"boost::filesystem not found"**
   - Run `make -j boost` manually (takes 10-15 min)
   - Or install system boost: `apt-get install libboost-all-dev`

3. **VLA extension warnings causing build failures**
   - Add required flags to CONFIG.mine:
   ```bash
   echo "CXXFLAGS += -Wno-vla-extension -Wno-error=vla-extension" >> CONFIG.mine
   echo "CFLAGS += -Wno-vla-extension -Wno-error=vla-extension" >> CONFIG.mine
   ```

4. **Slow compilation (>1 hour)**
   - Always use `-j` flag: `make -j target.x` or `make -j$(nproc)`
   - Add to CONFIG.mine: `MY_CFLAGS += -DFEWER_PRIMES -DFEWER_RINGS -DCOMP_SEC=10`

5. **Runtime: "Connection refused"**
   - Ensure all parties use same `-h` (hostname) and `-pn` (port)
   - Check firewall rules if running across machines
   - Verify party IDs are 0, 1, 2 (not duplicated)

6. **"xxhash/blake3 not found" (ECHO protocol)**
   - Install required libraries: `sudo apt-get install libxxhash-dev libblake3-dev`
   - These are essential for ECHO protocol implementation

7. **"CONFIG.mine: Permission denied"**
   - Always run `touch CONFIG.mine` before first build
   - Ensure write permissions in repository root

## Important Notes for Agents

- **ALWAYS run `make tldr` once for initial setup** - this is a ONE-TIME operation, NOT needed before each compilation
- **ALWAYS use `-j` flag for parallel compilation** - e.g., `make -j target.x` for faster builds
- **ALWAYS setup TensorFlow inference** - see README-MP_SPDZ.md for details
- **DO NOT commit** CONFIG.mine (user-specific), build artifacts (see .gitignore section above)
- **Primary documentation:** README-MP_SPDZ.md is the authoritative guide; README.md may be outdated
- **Required build targets:** Focus on `replicated-ring-party.x`, `sy-rep-ring-party.x`, `echo-*`, `covert-mac-party.x` for this research
- **Protocol changes:** Edit `Protocols/*.h`, then rebuild affected `*-party.x` with `make -j`
- **New benchmarks:** Add to `Utils/`, follow pattern of mul-benchmark.cpp

---

**For authoritative MP-SPDZ documentation:** See `README-MP_SPDZ.md` (50+ KB comprehensive guide)  
**For protocol details:** See paper "ECHO: Efficient Covertly-Secure Three-party Computation with Public Verifiability"  
**Note:** README.md may contain outdated information; always prefer README-MP_SPDZ.md for build instructions
