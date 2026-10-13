#define RING_SIZE 52

#include "Math/gfp.hpp"
#include "Processor/RingMachine.hpp"
#include "Machines/Rep.hpp"
#include "Processor/RingOptions.h"
#include "Math/Integer.h"
#include "Machines/RepRing.hpp"

#include "Protocols/BGIN19MC.hpp"
#include "Protocols/CovertShare.h"

thread_local size_t g_thread_bytes_written = 0;

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    HonestMajorityRingMachine<DzkpRingShare, DzkpShare>(argc, argv, opt);
}
