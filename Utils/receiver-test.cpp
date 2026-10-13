#include "Tools/random.h"
#include "Networking/CryptoPlayer.h"
#include <fstream>

using namespace std;

int main() {
    Names N(0, 2, "localhost", 12345);
    CryptoPlayer P(N);

    ifstream input("test.data");
    if (!input.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    cout << "Connected." << endl;

    octetStream os;
    string str;

    while (true) {
        os.reset_write_head();
        os.input(input);
        os.get(str);
        cout << str << endl;
        if (str == "exit") {
            break;
        }
    }

    return 0;
}