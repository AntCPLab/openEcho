#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>

#include <NTL/GF2E.h>
#include <NTL/GF2X.h>

using namespace std;
using namespace NTL;

namespace {

GF2X makeAesPoly() {
    // x^8 + x^4 + x^3 + x + 1 (0x11B)
    GF2X P;
    SetCoeff(P, 8);
    SetCoeff(P, 4);
    SetCoeff(P, 3);
    SetCoeff(P, 1);
    SetCoeff(P, 0);
    return P;
}

GF2X makePolyFromHex(uint32_t hex) {
    // Build polynomial from bits of hex, bit i => coeff for x^i
    GF2X P;
    for (unsigned i = 0; i < 32; ++i) {
        if ((hex >> i) & 1U) SetCoeff(P, i);
    }
    return P;
}

struct Options {
    uint64_t iterations = 20000000; // default 20M mults
    uint64_t warmup = 2000000;      // default 2M warmup
    size_t   poolSize = 32768;      // number of pre-generated operands
    string   poly = "aes";         // "aes" or hex like 0x11B
};

Options parseOptions(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        string a(argv[i]);
        auto getVal = [&](const string& key) -> const char* {
            if (a.rfind(key + "=", 0) == 0) return a.c_str() + key.size() + 1;
            return nullptr;
        };
        if (auto v = getVal("--iters")) opt.iterations = strtoull(v, nullptr, 10);
        else if (auto v = getVal("--warmup")) opt.warmup = strtoull(v, nullptr, 10);
        else if (auto v = getVal("--pool")) opt.poolSize = strtoull(v, nullptr, 10);
        else if (auto v = getVal("--poly")) opt.poly = v;
        else if (a == "-h" || a == "--help") {
            cout << "Usage: ntl-gf2e8-bench.x [--iters=N] [--warmup=N] [--pool=N] [--poly=aes|0xHEX]\n";
            exit(0);
        }
    }
    if (opt.poolSize == 0) opt.poolSize = 1;
    return opt;
}

GF2X selectPolynomial(const string& polyArg) {
    if (polyArg == "aes") return makeAesPoly();
    // Expect format 0x... (e.g., 0x11B)
    if (polyArg.rfind("0x", 0) == 0 || polyArg.rfind("0X", 0) == 0) {
        uint32_t v = static_cast<uint32_t>(strtoul(polyArg.c_str(), nullptr, 16));
        return makePolyFromHex(v);
    }
    // Fallback: treat as decimal bitmask
    uint32_t v = static_cast<uint32_t>(strtoul(polyArg.c_str(), nullptr, 0));
    return makePolyFromHex(v);
}

} // namespace

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);

    const Options opt = parseOptions(argc, argv);
    const GF2X P = selectPolynomial(opt.poly);
    GF2E::init(P);

    cout << "NTL GF(2^8) multiplication benchmark\n";
    cout << "- modulus P(x) = ";
    cout << P << "\n";
    cout << "- iterations  = " << opt.iterations << "\n";
    cout << "- warmup      = " << opt.warmup << "\n";
    cout << "- pool size   = " << opt.poolSize << "\n";

    // Pre-generate operands to avoid RNG costs in timed loop
    vector<GF2E> a(opt.poolSize), b(opt.poolSize);
    for (size_t i = 0; i < opt.poolSize; ++i) {
        random(a[i]);
        random(b[i]);
    }

    GF2E acc; // accumulated result; printed to prevent optimization

    // Warmup
    for (uint64_t i = 0; i < opt.warmup; ++i) {
        const GF2E& x = a[i % opt.poolSize];
        const GF2E& y = b[i % opt.poolSize];
        acc += x * y;
    }

    // Timed run
    auto t0 = chrono::high_resolution_clock::now();
    for (uint64_t i = 0; i < opt.iterations; ++i) {
        const GF2E& x = a[i % opt.poolSize];
        const GF2E& y = b[i % opt.poolSize];
        acc += x * y;
    }
    auto t1 = chrono::high_resolution_clock::now();

    const double seconds = chrono::duration<double>(t1 - t0).count();
    const double ns_per_op = (seconds * 1e9) / static_cast<double>(opt.iterations);
    const double mops = static_cast<double>(opt.iterations) / seconds / 1e6;

    cout << "Elapsed: " << seconds << " s\n";
    cout << "Throughput: " << mops << " M mul/s\n";
    cout << "Latency: " << ns_per_op << " ns/op\n";
    // Use sink so compiler keeps the loop
    cout << "Accumulator (ignore): " << acc << "\n";

    return 0;
}


