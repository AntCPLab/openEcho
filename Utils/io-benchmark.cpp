#include "Tools/random.h"
#include "Tools/time-func.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

static constexpr uint64_t MiB = 1024ull * 1024ull;
static constexpr uint64_t GiB = 1024ull * 1024ull * 1024ull;

static void usage(const char* argv0)
{
    cerr << "Usage: " << argv0 << " <output-file>\n"
         << "Generates 10GiB total random data in 1MiB chunks using Tools/random.h,\n"
         << "writes via ofstream, and measures elapsed time with Tools/time-func.h.\n";
}

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            usage(argv[0]);
            return 1;
        }

        const string outPath = argv[1];
        const uint64_t chunkBytes = 1ull * MiB;
        const uint64_t totalBytes = 10ull * GiB;

        if (chunkBytes > static_cast<uint64_t>(numeric_limits<int>::max()))
            throw runtime_error("chunkBytes too large for PRNG::get_octets(int len)");

        vector<octet> buffer(chunkBytes);
        SeededPRNG prng;

        ofstream out(outPath, ios::binary | ios::out | ios::trunc);
        out.exceptions(ios::badbit | ios::failbit);

        prng.get_octets(buffer.data(), static_cast<int>(buffer.size()));

        Timer timer;
        timer.start();

        uint64_t written = 0;
        uint64_t nextProgress = 1ull * GiB;
        while (written < totalBytes)
        {
            out.write(reinterpret_cast<const char*>(buffer.data()),
                      static_cast<streamsize>(buffer.size()));
            written += chunkBytes;

            if (written >= nextProgress)
            {
                cerr << "written " << (written / GiB) << " / " << (totalBytes / GiB)
                     << " GiB\r" << flush;
                nextProgress += 1ull * GiB;
            }
        }

        out.flush();
        timer.stop();

        const double seconds = timer.elapsed();
        const double totalGiB = static_cast<double>(totalBytes) / static_cast<double>(GiB);
        const double gibPerSecond = totalGiB / seconds;

        cerr << "\n";
        cout << fixed << setprecision(3)
             << "Wrote " << totalGiB << " GiB in " << seconds << " s"
             << " (" << gibPerSecond << " GiB/s)\n";

        return 0;
    }
    catch (const exception& e)
    {
        cerr << "io-benchmark: error: " << e.what() << "\n";
        return 2;
    }
}
