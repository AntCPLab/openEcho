/*
 * astra-prep-party.cpp
 *
 */

// #define COUNT_INSTRUCTIONS
// #define TIME_INSTRUCTIONS

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

#include "Protocols/EchoShare.h"
#include "Protocols/EchoInput.hpp"

int main(int argc, const char** argv)
{
    HonestMajorityRingMachine<EchoPrepShare2, EchoPrepShare>(argc, argv);
}
