#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"
#include "version/version.h"

using namespace std;

string sourceName2funcName(string sourceName) {
    replace(sourceName.begin(), sourceName.end(), '.', '_');
    replace(sourceName.begin(), sourceName.end(), '/', '_');
    replace(sourceName.begin(), sourceName.end(), '-', '_');
    return sourceName;
}

void emit_header(vector<string> sourceFiles, ostream &out) {
    out << "#include<string>" << endl << "#include<vector>" << endl;
    out << "namespace ruby_typer{" << endl << "namespace rbi{" << endl;
    for (auto &file : sourceFiles) {
        out << "  std::string " + sourceName2funcName(file) << "();" << endl;
    }
    out << "  std::vector<std::pair<std::string, std::string>> all();" << endl;

    out << "}};" << endl;
}

void emit_classfile(vector<string> sourceFiles, ostream &out) {
    out << "#include<string>" << endl << "#include<vector>" << endl;
    out << "namespace ruby_typer{" << endl << "namespace rbi{" << endl;
    for (auto &file : sourceFiles) {
        out << "  std::string " + sourceName2funcName(file) << "() {" << endl;
        out << "  return \"" + absl::CEscape(ruby_typer::File::read(file.c_str())) + "\";" << endl << "}" << endl;
    }
    out << "std::vector<std::pair<std::string, std::string>> all() {" << endl;
    out << "  std::vector<std::pair<std::string, std::string>> result;" << endl;
    for (auto &file : sourceFiles) {
        string version = ruby_typer::Version::build_scm_revision;
        if (version == "0") {
            version = "master";
        }
        string permalink = "https://git.corp.stripe.com/stripe-internal/ruby-typer/tree/" + version + "/" + file;
        out << "  result.push_back(std::make_pair<std::string, std::string>(\"" + absl::CEscape(permalink) + "\", " +
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

        vector<string> sources;
        for (int i = 3; i < argc; i++) {
            sources.emplace_back(argv[i]);
        }

        emit_header(sources, header);
        emit_classfile(sources, classfile);
    }

    return 0;
}
