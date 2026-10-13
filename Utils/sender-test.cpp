#include "Tools/random.h"
#include "Networking/CryptoPlayer.h"
#include <fstream>

using namespace std;

int main() {
    Names N(1, 2, "localhost", 12345);
    CryptoPlayer P(N);

    ofstream output("test.data");
    if (!output.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    cout << "Connected." << endl;

    octetStream os;
    string str;

    while (true) {
        os.reset_write_head();
        cout << "Enter data to send (type 'exit' to quit): ";
        cin >> str;
        os.store(str);
        os.output(output);
        if (str == "exit") {
            break;
        }
    }

    return 0;
}