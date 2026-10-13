#define RING_SIZE 52

#include "Math/gfp.hpp"
#include "Processor/RingMachine.hpp"
#include "Machines/Rep.hpp"
#include "Protocols/UssCcsShare.h"
#include "Protocols/UssCcsRingShare.h"
#include "Processor/RingOptions.h"
#include "Math/Integer.h"
#include "Machines/RepRing.hpp"

thread_local size_t g_thread_bytes_written = 0;

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HonestMajorityRingMachine<UssCcsRingShare, UssCcsShare>(argc, argv, opt);
}
