#include <iostream>
#include <fstream>
#include "Tools/PointerVector.h"
#include "Math/Z2k.hpp"
#include "Math/BitVec.h"
#include "Processor/Instruction.hpp"

using namespace std;

int main() {
    
    using T = gf2n_short;
    T::reset();
    T::init_field(8);

    T a, b;
    T c = a * b;

    cout << c << endl;

    return 0;
}