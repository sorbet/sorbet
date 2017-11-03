#include "../../common/common.h"
#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string sourceName2funcName(string sourceName) {
    std::replace(sourceName.begin(), sourceName.end(), '.', '_');
    std::replace(sourceName.begin(), sourceName.end(), '/', '_');
    return sourceName;
}

void emit_header(std::vector<std::string> sourceFiles, ostream &out) {
    out << "#include<string>" << endl << "#include<vector>" << endl;
    out << "namespace ruby_typer{" << endl << "namespace rbi{" << endl;
    for (auto &file : sourceFiles) {
        out << "  std::string " + sourceName2funcName(file) << "();" << endl;
    }
    out << "  std::vector<std::pair<std::string, std::string>> all();" << endl;

    out << "}};" << endl;
}

string escape(string what) {
    char escaped[] = {'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\"'};
    char non_escaped[] = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\"'};
    static_assert(sizeof(escaped) == sizeof(non_escaped), "???");
    stringstream buf;
    for (char c : what) {
        int j;
        for (j = 0; j < sizeof(escaped); j++) {
            if (c == escaped[j]) {
                buf << '\\';
                buf << non_escaped[j];
                break;
            }
        }
        if (j == sizeof(escaped))
            buf << c;
    }

    return buf.str();
}

void emit_classfile(std::vector<std::string> sourceFiles, ostream &out) {
    out << "#include<string>" << endl << "#include<vector>" << endl;
    out << "namespace ruby_typer{" << endl << "namespace rbi{" << endl;
    for (auto &file : sourceFiles) {
        out << "  std::string " + sourceName2funcName(file) << "() {" << endl;
        out << "  return \"" + escape(ruby_typer::File::read(file.c_str())) + "\";" << endl << "}" << endl;
    }
    out << "std::vector<std::pair<std::string, std::string>> all() {" << endl;
    out << "  std::vector<std::pair<std::string, std::string>> result;" << endl;
    for (auto &file : sourceFiles) {
        out << "  result.push_back(std::make_pair<std::string, std::string>(\"" + escape(file) + "\", " +
                   sourceName2funcName(file) + "()));"
            << endl;
    }
    out << "  return result;" << endl;

    out << "}}};" << endl;
}

int main(int argc, char **argv) {
    // emit header file
    {
        ofstream header(argv[1], ios::trunc);
        if (!header.good()) {
            cerr << "unable to open " << argv[1] << endl;
            return 1;
        }

        ofstream classfile(argv[2], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << endl;
            return 1;
        }

        std::vector<std::string> sources;
        for (int i = 3; i < argc; i++) {
            sources.emplace_back(argv[i]);
        }

        emit_header(sources, header);
        emit_classfile(sources, classfile);
    }

    return 0;
}
