/*
 * replicated-ring-party.cpp
 *
 */

// #define TIME_INSTRUCTIONS
// #define COUNT_INSTRUCTIONS
// #define RING_SIZE 52

#include "Protocols/Rep3Share2k.h"
#include "Processor/RingOptions.h"
#include "Math/Integer.h"
#include "Machines/RepRing.hpp"
#include "Processor/RingMachine.hpp"

// Thread-local variable for accumulating bytes written statistics
thread_local size_t g_thread_bytes_written = 0;

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<Rep3Share2, Rep3Share>(argc, argv);
}
