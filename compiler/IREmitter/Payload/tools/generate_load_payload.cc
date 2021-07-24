#include "common/FileOps.h"
#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
int main(int argc, char **argv) {
    {
        ofstream out(argv[2], ios::trunc);
        auto data = sorbet::FileOps::read(argv[1]);
        out << "#include<string_view>" << '\n' << "\nusing namespace std;\n";
        out << "namespace sorbet{" << '\n' << "namespace compiler{\n";
        out << "static char actualData[] = {";
        for (int i = 0; i < data.length(); i++) {
            out << (int)data[i] << ", ";
        }
        out << "};\n string_view getDefaultModuleBitcode() {\n  return string_view(&actualData[0], " << data.length()
            << ");\n}\n};\n};\n";
    }

    return 0;
}
