#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

vector<uint8_t> readBinary(char *path) {
    ifstream fin{path, ios::binary};
    if (!fin.good()) {
        throw sorbet::FileNotFoundException(string(path));
    }

    vector<uint8_t> data;
    fin.seekg(0, ios::end);
    size_t filesize = fin.tellg();
    fin.seekg(0, ios::beg);

    data.resize(filesize);

    fin.read((char *)data.data(), filesize);

    return data;
}

void writeConstant(ofstream &out, string_view name, const vector<uint8_t> data) {
    out << "const uint8_t " << name << "[] = {";
    bool first = true;
    int i = -1;
    for (auto c : data) {
        ++i;
        if (!first) {
            out << ",";
        }
        first = false;
        if (i % 10 == 0) {
            out << endl << "    ";
        } else {
            out << " ";
        }
        out << (int)c;
    }
    out << endl << "};" << endl;
}

int main(int argc, char **argv) {
    auto symtabData = readBinary(argv[1]);
    auto nametabData = readBinary(argv[2]);
    auto filetabData = readBinary(argv[3]);

    ofstream classfile(argv[4], ios::trunc);

    classfile << "#include \"common/common.h\"" << endl;
    classfile << "namespace {" << endl;

    writeConstant(classfile, "symbolTablePayload", std::move(symtabData));
    writeConstant(classfile, "nameTablePayload", std::move(nametabData));
    writeConstant(classfile, "fileTablePayload", std::move(filetabData));

    classfile << "}" << endl;

    classfile << "extern const uint8_t * const PAYLOAD_SYMBOL_TABLE = (const uint8_t * const)&symbolTablePayload;"
              << endl;
    classfile << "extern const uint8_t * const PAYLOAD_NAME_TABLE = (const uint8_t * const)&nameTablePayload;" << endl;
    classfile << "extern const uint8_t * const PAYLOAD_FILE_TABLE = (const uint8_t * const)&fileTablePayload;" << endl;
    classfile << "extern const bool PAYLOAD_EMPTY = false;" << endl;

    classfile.close();

    return 0;
}
