/*
 * astra-prep-party.cpp
 *
 */

// #define COUNT_INSTRUCTIONS
// #define TIME_INSTRUCTIONS

#define RING_SIZE 52

#include "Protocols/AstraShare.h"
#include "GC/AstraSecret.h"

#include "Processor/Machine.hpp"
#include "Processor/RingMachine.hpp"
#include "Protocols/ReplicatedMC.hpp"
#include "Protocols/AstraInput.hpp"
#include "Protocols/AstraPrep.hpp"
#include "Protocols/Astra.hpp"
#include "Protocols/Trio.hpp"
#include "GC/ShareSecret.hpp"

// Thread-local variable for accumulating bytes written statistics
thread_local size_t g_thread_bytes_written = 0;

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<AstraPrepShare2, AstraPrepShare>(argc, argv);
}
