# Echo Protocol POpen Deadlock Issue

## Problem Summary

The Echo protocol (`echo-party.x`) hangs after completing training when running `mnist_full_D` with `trunc_pr split` options, while it works (with incorrect results) on `mnist_full_A/B/C` and works correctly with `replicated-ring-party.x`.

**Affected Command:**
```bash
./compile.py -R 64 mnist_full_D 1 128 16 2dense trunc_pr split
./Scripts/echo-online.sh mnist_full_D-1-128-16-2dense-trunc_pr-split
```

**Symptom:** Program hangs after printing:
```
Stopped timer 1 at 42.5054 (2235.83 MB, 1418256 rounds)
test loss: -0.573508
acc: 0.0435 (435/10000)
```

## Root Cause

**Correctness bug in `EchoMC<T>::POpen()` (Protocols/EchoMC.hpp:101-139)**

The Echo protocol's reveal (POpen) implementation has a bug where **different parties reconstruct different revealed values**:

```cpp
void EchoMC<T>::POpen(vector<typename T::open_type>& values, const vector<T>& S, const Player& P) {
    int my_num = P.my_num();
    if (my_num == 0) {
        // P0: reads from preprocessing
        memcpy(values.data(), prep_os.get_data(), ...);
        P.pass_around(prep_os, recv_os, 1);
    }
    else if (my_num == 1) {
        // P1: values = S[i][0]
        values[i] = S[i][0];
        S[i][1].pack(this->os);
        P.pass_around(this->os, recv_os, 1);
    }
    else {
        // P2: values = S[i].sum()
        values[i] = S[i].sum();
        S[i][0].pack(this->os);
        P.pass_around(this->os, recv_os, 1);
    }
    // All add received value
    values[i] += recv_os.get_no_check<typename T::open_type>();
}
```

**P1 uses a different reconstruction formula** than P0/P2, causing all parties to see different revealed values.

## How the Deadlock Occurs

1. After training completes, the ML optimizer reveals `n_correct` (number of correct predictions) via POpen
2. The code checks: `if (n_correct < threshold)` → reduce learning rate
3. This compiles to a `JMPEQZ` (conditional jump) instruction
4. **Due to the POpen bug:**
   - P0/P2 see `n_correct` as **non-zero** → condition FALSE → don't jump
   - P1 sees `n_correct` as **zero** → condition TRUE → jumps to different code path
5. **Parties execute different code paths** → deadlock when trying to synchronize

### Instruction Trace Showing Divergence

**P0 (after accuracy print):**
```
[TRACE P0] opcode=146 (0x92)   # JMPEQZ - conditional jump
[TRACE P0] opcode=3 (0x3)      # LDSI - didn't jump, continued
[TRACE P0] opcode=145 (0x91)   # Many JMPNZ (loop)
...
[TRACE P0] *** RUN_TAPE reached, trace complete ***
```

**P1 (after accuracy print):**
```
[TRACE P1] opcode=146 (0x92)   # JMPEQZ - conditional jump
[TRACE P1] opcode=191 (0xbf)   # CONDPRINTSTR - jumped to different path!
[TRACE P1] opcode=191 (0xbf)
...
[TRACE P1] opcode=233 (0xe9)   # ACTIVE - stuck, never reaches RUN_TAPE
```

**P2 (same as P0):**
```
[TRACE P2] opcode=146 (0x92)   # JMPEQZ
[TRACE P2] opcode=3 (0x3)      # LDSI - didn't jump
...
[TRACE P2] *** RUN_TAPE reached, trace complete ***
```

P1 takes a different branch → P0/P2 wait for P1 at tape join → **deadlock**

## Why mnist_full_A/B/C Don't Deadlock

The bug affects **all** Echo runs, but:

- **mnist_full_A**: Wrong results (10% accuracy vs 93% expected), but by luck all parties took the same branch in all JMPEQZ instructions → no deadlock
- **mnist_full_D**: Wrong results + unlucky that P1's wrong `n_correct` value crossed the threshold differently than P0/P2 → **deadlock**

The difference is **pure chance** based on:
- Network architecture (Conv2D stride, layer sizes)
- Specific revealed values during execution
- Whether the buggy values happen to cross conditional thresholds differently

## Workaround

**Compile with `no_acc` to skip accuracy checking:**

```bash
./compile.py -R 64 mnist_full_D 1 128 16 2dense trunc_pr split no_acc
./Scripts/echo-online.sh mnist_full_D-1-128-16-2dense-trunc_pr-split-no_acc
```

This avoids the POpen reveal of `n_correct` and the problematic JMPEQZ branch entirely.

**Benefits:**
- ✅ No deadlock
- ✅ Accurate timing/communication benchmarks
- ✅ Training still runs normally
- ❌ No accuracy output (but not needed for performance benchmarking)

## Long-term Fix Required

The Echo protocol's `POpen` implementation needs correction to ensure **all parties reconstruct identical revealed values**. The reconstruction formula for P1 appears incorrect relative to P0/P2.

This is a **critical correctness bug** that affects all Echo protocol executions, not just mnist_full_D.

## Debugging Process

1. Added debug output to `Machine::run()`, `join_tape()`, `Instruction::execute()`
2. Found P1 stops executing before final `RUN_TAPE` instruction
3. Added instruction tracing after OPEN (reveal) operations
4. Identified divergence at `JMPEQZ` (opcode 0x92) - conditional branch
5. Traced back to `n_correct` reveal in ML optimizer's learning rate reduction logic (Compiler/ml.py:2800-2811)
6. Confirmed POpen returns different values to different parties

## Files Modified (Debug Code - Can Be Reverted)

- `Processor/Machine.hpp`: Added debug output for tape join operations
- `Processor/Instruction.hpp`: Added debug output for OPEN, MULS, DOTPRODS, RUN_TAPE, JOIN_TAPE instructions + instruction tracing
- `Protocols/EchoMC.hpp`: Added debug output for FinalCheck (not reached in deadlock)

## Impact Assessment

**Severity:** HIGH - Correctness bug affecting all Echo protocol executions

**Affected:**
- All programs using Echo protocol with conditional branches after reveals
- Particularly affects ML training with accuracy checking enabled
- Results in either deadlocks or silent correctness failures

**Not Affected:**
- Replicated-ring-party.x (ABY3) - works correctly
- Programs compiled with `no_acc` flag - avoids problematic reveals

## References

- Instruction opcodes: `Processor/Instruction.h`
- ML optimizer logic: `Compiler/ml.py:2690-2821`
- Echo POpen implementation: `Protocols/EchoMC.hpp:101-139`
- JMPEQZ = 0x92 (146) - Jump if equal to zero
- CONDPRINTSTR = 0xBF (191) - Conditional print string
