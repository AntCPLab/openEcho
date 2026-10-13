#include "Protocols/MalRepRingOptions.cpp"
#include "Processor/OnlineOptions.hpp"

int main(int argc, const char** argv) {
    ez::ezOptionParser opt;
    OnlineOptions online_opts;
    online_opts.finalize(opt, argc, argv);
    OnlineOptions::singleton = online_opts;

    cout << "Batch size = " << OnlineOptions::singleton.batch_size << endl;
    cout << "Bucket size = " << OnlineOptions::singleton.bucket_size << endl;
    cout << "Lambda = " << OnlineOptions::singleton.lambda << endl;

    return 0;
}