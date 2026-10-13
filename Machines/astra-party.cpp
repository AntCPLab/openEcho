/*
 * astra-party.cpp
 *
 */

// #define COUNT_INSTRUCTIONS
// #define TIME_INSTRUCTIONS

#define RING_SIZE 52

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/AstraMC.hpp"
#include "Protocols/Astra.hpp"
#include "GC/ShareSecret.hpp"

// Thread-local variable for accumulating bytes written statistics
thread_local size_t g_thread_bytes_written = 0;

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<AstraShare2, AstraShare>(argc, argv, 2);
}
