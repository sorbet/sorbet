#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    ifstream fin(argv[1], ios::binary);
    if (!fin.good()) {
        throw sorbet::FileNotFoundException();
    }
    vector<sorbet::u1> data;

    fin.seekg(0, ios::end);
    size_t filesize = fin.tellg();
    fin.seekg(0, ios::beg);

    data.resize(filesize);

    fin.read((char *)data.data(), filesize);

    ofstream classfile(argv[2], ios::trunc);

    classfile << "#include \"common/common.h\"\n"
              << "\n"
              << "const sorbet::u1 nameTablePayload[] = {\n";
    int i = -1;
    bool first = true;
    for (auto c : data) {
        ++i;
        if (!first) {
            classfile << ", ";
        }
        first = false;
        if (i % 10 == 0) {
            classfile << "\n    ";
        }
        classfile << (int)c;
    }
    classfile << "};\n";
    classfile << "extern const sorbet::u1 * const getNameTablePayload = (const sorbet::u1 * const)&nameTablePayload;"
              << '\n';

    classfile.close();

    return 0;
}
