#include "../../common/common.h"
#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
    // emit header file
    ifstream fin(argv[1]);
    if (!fin.good()) {
        throw ruby_typer::FileNotFoundException();
    }
    // Determine the file length
    fin.seekg(0, ios::end);
    vector<ruby_typer::u4> data(fin.tellg());
    fin.seekg(0, ios::beg);
    fin.read((char *)data.data(), data.size());

    ofstream classfile(argv[2], ios::trunc);

    classfile << "#include \"common/common.h\"\n"
              << "\n"
              << "const ruby_typer::u4 nameTablePayload[] = {\n";
    int i = 0;
    bool first = true;
    for (auto c : data) {
        if (!first) {
            classfile << ", ";
        }
        first = false;
        if (i % 10 == 0) {
            classfile << "\n    ";
        }
        classfile << c;
        ++i;
    }
    classfile << "};\n";
    classfile
        << "extern const ruby_typer::u4 * const getNameTablePayload = (const ruby_typer::u4 * const)&nameTablePayload;"
        << endl;

    return 0;
}
