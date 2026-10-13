# Echo Protocol Framework Summary

## 1. Protocol Architecture Overview

### 1.1 Share Structure (Astra Format)
The Echo protocol uses the **Astra share format** for 3-party computation:

| Party | Shares Held | Role |
|-------|-------------|------|
| P0 | `{a1, a2}` | Dealer/Generator |
| P1 | `{a0, a1}` | Compute Player |
| P2 | `{a0, a2}` | Compute Player |

**Secret reconstruction**: `x = a0 + a1 + a2`

### 1.2 Class Hierarchy

```
Echo Protocol Classes:
├── EchoShare<T> : AstraShare<T>
│   ├── Protocol = Echo<T>
│   ├── MAC_Check = EchoMC<This>
│   ├── LivePrep = EchoPrep<This>
│   └── Input = EchoInput<This>
│
├── EchoPrepShare<T> : AstraPrepShare<T>
│   ├── Protocol = EchoPrepProtocol<This>
│   ├── MAC_Check = EchoPrepMC<This>
│   ├── LivePrep = EchoPrepPrep<This>
│   └── Input = EchoPrepInput<This>
│
├── Echo<T> : AstraOnlineBase<T> : AstraBase<T> : ProtocolBase<T>
│   └── Online phase protocol execution
│
└── EchoPrepProtocol<T> : AstraPrepProtocol<T> : AstraBase<T> : ProtocolBase<T>
    └── Preprocessing phase protocol execution
```

### 1.3 Binary Protocol Classes

```
Binary Protocol Classes:
├── EchoBin<T> : ProtocolBase<T>
│   └── Online binary operations
│
└── EchoPrepBinProtocol<T> : ReplicatedBase, ProtocolBase<T>
    └── Preprocessing binary operations
```

### 1.4 Execution Flow

```
Offline Phase (echo-prep-party):
  HonestMajorityRingMachine<EchoPrepShare2, EchoPrepShare>
  └── Generates preprocessing data → Player-Data/

Online Phase (echo-party):
  HonestMajorityRingMachine<EchoShare2, EchoShare>
  └── Reads preprocessing data from Player-Data/
```

---

## 2. Truncation Protocol Structure

### 2.1 Call Stack

```
trunc_pr() Entry Point
│
├── TruncPrTupleList<T> infos(regs, S, size)  // Parse parameters
│   ├── dest_base: destination register
│   ├── source_base: source register  
│   ├── k: bit-width of input
│   ├── m: truncation amount (shift right by m bits)
│   └── Determines big_gap vs small_gap
│
├── if infos.have_big_gap():
│   └── trunc_pr_big_gap(infos.get_big_gap(), size, proc)
│
└── if infos.have_small_gap():
    └── trunc_pr_small_gap(infos.get_small_gap(), size, proc)
```

### 2.2 Big Gap vs Small Gap Determination

```cpp
// From TruncPrTuple.h
big_gap_ = (k <= T::n_bits() - OnlineOptions::singleton.trunc_error);

// Example: 64-bit ring with trunc_error=40
// big_gap if k <= 24 bits
// small_gap if k > 24 bits
```

### 2.3 Big Gap Truncation (Simple Case)

**Preprocessing Phase (`AstraPrepProtocol::trunc_pr_big_gap`)**:
```
P0 (Generator):
  1. Compute: sum = -(-x.neg_lambda_sum()) >> m
  2. Generate random r via shared PRNG with P2
  3. y[0] = sum - r, y[1] = r
  4. Send y[0] to P1

P1 (Compute):
  1. Receive y.neg_lambda from P0
  2. Store to preprocessing file

P2:
  1. Generate same r via shared PRNG with P0
  2. Store to preprocessing file
```

**Online Phase (`AstraOnlineBase::trunc_pr_big_gap`)**:
```
All parties:
  1. Read precomputed truncation shares from preprocessing file
  2. Set y.common_m() = x.common_m() >> m
```

### 2.4 Small Gap Truncation (Complex Case)

**Call Stack**:
```
trunc_pr_small_gap()
├── init_reduced_mul(size * infos.size())
├── init_input0(size * infos.size())
│
├── For each truncation:
│   ├── T::pre_input0<T, MY_NUM>(r >> m, protocol)  // Generator shares r>>m
│   └── T::pre_reduced_mul<T, MY_NUM>(...)          // MSB correction multiplication
│
├── exchange_reduced_mul()
├── exchange_input0()
│
└── For each truncation:
    ├── r_prime = T::post_input0<T, MY_NUM>(protocol)
    ├── res = T::post_reduced_mul<T, MY_NUM>(protocol)
    └── x += r_prime + correction_shift(res.second)
```

**Algorithm Overview**:
1. Add bias: `c = x + 2^(k-1) - 1`
2. Share `c >> m` from generator
3. Extract MSB for overflow correction
4. Compute correction: `correction = 2^(n-m) * (r_msb XOR c_msb - r_msb * c_msb)`
5. Final result: `y = (c >> m) - 2^(k-m-1) + correction`

---

## 3. Key Protocol Components

### 3.1 Important Constants

```cpp
// In AstraBase<T>
static const int gen_player = 0;   // P0 is generator
static const int comp_player = 1;  // P1 is compute player
```

### 3.2 Share Access Methods

```cpp
// For EchoShare/AstraShare:
x[0], x[1]           // Direct component access
x.sum()              // Returns a0 + a1 + a2 (reconstructed value, only valid for P0)
x.neg_lambda(i)      // Returns -lambda component for party i
x.neg_lambda_sum(i)  // Returns sum of neg_lambda components
x.common_m()         // Returns the common 'm' component
x.m(i)               // Returns m component for index i
```

### 3.3 File I/O Pattern

**Preprocessing stores to**:
```
Player-Data/<share-type>-<n_players>-<preprocessing>/Protocol<suffix>-P<my_num>-T<thread_num>
```

**Online reads from**: Same path structure

### 3.4 MAC Handling

The Echo protocol uses MAC for verification:
```cpp
// In EchoPrepProtocol::init()
alpha = this->MC->get_alpha();  // Get MAC key shares
key = alpha[0] + alpha[1];       // Combined key (for P0)
```

---

## 4. Binary Split Operation

### 4.1 Index Mapping for Split

```cpp
// Maps arithmetic share index to Wallace tree column
const int index_mapping[3][3] = {
    {1, 2, 0},  // P0: i=0(a1)->col1, i=1(a2)->col2
    {0, 1, 2},  // P1: i=0(a0)->col0, i=1(a1)->col1
    {0, 2, 1}   // P2: i=0(a0)->col0, i=1(a2)->col2
};
```

### 4.2 Split Function

```cpp
template<class U, class V>
static void split(StackedVector<U>& dest,
        const vector<int>& regs, int n_bits, const V* source,
        int n_inputs, typename U::Protocol& protocol) {
    Rep3Share2<T::N_BITS>::split(dest, regs, n_bits, source, n_inputs, protocol);
}
```

---

## 5. Important Flags and Properties

```cpp
// In EchoShare<T>
static const bool dishonest_majority = true;
static const bool variable_players = false;
static const bool has_trunc_pr = true;        // Truncation is supported
static const bool function_dependent = true;
```

---

## 6. Multiplication Protocol Structure

### 6.1 Preprocessing Multiplication (`EchoPrepProtocol::exchange_mul`)

```
P0:
  1. Compute rxy = x.sum() * y.sum()
  2. Generate random shares via PRNGs
  3. Send {rxy_2, alpha_x - rand, alpha_y - rand} to P2
  4. Add MAC verification data

P1:
  1. Use shared PRNG to generate random values
  2. Store preprocessing data

P2:
  1. Receive from P0
  2. Compute local shares
  3. Store preprocessing data
```

### 6.2 Online Multiplication (`Echo::exchange_mul`)

```
All parties:
  1. Read preprocessing triple from file
  2. Compute z = xy using Beaver triple: z = c + (a-x)(b-y) + a*y + b*x
  3. Exchange shares as needed
```

---

## 7. PRNG Structure

```cpp
// Shared PRNGs between parties
array<PRNG, 2> shared_prngs;

// Access via:
prng_protocol.shared_prngs[0]  // Shared with previous party
prng_protocol.shared_prngs[1]  // Shared with next party
```

---

## 8. Writing Custom Truncation

To implement custom truncation in Echo:

1. **Add method declaration** in `Echo.h`:
```cpp
template<int = 0>
void trunc_pr(const vector<int>& regs, int size, SubProcessor<T>& proc, false_type);
```

2. **Implement in `Echo.hpp`**:
```cpp
template<class T>
template<int>
void Echo<T>::trunc_pr(const vector<int>& regs, int size,
        SubProcessor<T>& proc, false_type)
{
    TruncPrTupleList<T> infos(regs, proc.get_S(), size);
    // Custom implementation
}
```

3. **Key parameters from `TruncPrTupleList`**:
   - `info.source_range`: Input shares `[[x]]`
   - `info.dest_range`: Output shares `[[x >> m]]`
   - `info.m`: Truncation amount
   - `info.k`: Input bit-width
   - `info.big_gap()` / `info.small_gap()`: Gap type

---

## 9. Build and Execution

```bash
# Compile
make echo-prep-party.x
make echo-party.x

# Run preprocessing (generates Player-Data/)
./echo-prep-party.x -p 0 <program> &
./echo-prep-party.x -p 1 <program> &
./echo-prep-party.x -p 2 <program>

# Run online phase
./echo-party.x -p 0 <program> &
./echo-party.x -p 1 <program> &
./echo-party.x -p 2 <program>
```