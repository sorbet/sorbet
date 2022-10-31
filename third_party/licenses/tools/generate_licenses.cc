#include "common/FileOps.h"
#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"
#include "absl/strings/strip.h"

using namespace std;

string sourceName2funcName(string sourceName) {
    absl::c_replace(sourceName, '.', '_');
    absl::c_replace(sourceName, '/', '_');
    absl::c_replace(sourceName, '-', '_');
    return sourceName;
}

string sourceName2DepName(string sourceName) {
    return (string)absl::StripPrefix(sourceName, ".txt");
}

void emitlicenses(vector<string> sourceFileNames, ostream &out) {
    out << "#include\"third_party/licenses/licenses.h\"\nusing namespace std;\n";
    out << "namespace sorbet::third_party::licenses {" << '\n';
    for (auto &file : sourceFileNames) {
        out << "  string_view " + sourceName2funcName(file) << "() {" << '\n';
        out << "  return \"" + absl::CEscape(sorbet::FileOps::read(file.c_str())) + "\"sv;" << '\n' << "}" << '\n';
    }
    out << "vector<pair<string_view, string_view> > all() {" << '\n';
    out << "  vector<pair<string_view, string_view> > result;" << '\n';
    for (auto &file : sourceFileNames) {
        out << "  result.emplace_back(make_pair<string_view, string_view>(\"" +
                   absl::CEscape(sourceName2DepName(file)) + "\"sv, " + sourceName2funcName(file) + "()));"
            << '\n';
    }
    out << "  return result;" << '\n';

    out << "}};" << '\n';
}

int main(int argc, char **argv) {
    // emit header file
    {
        ofstream outfile(argv[1], ios::trunc);
        if (!outfile.good()) {
            cerr << "unable to open " << argv[2] << '\n';
            return 1;
        }

        vector<string> sourceFileNames;
        for (int i = 2; i < argc; i++) {
            sourceFileNames.emplace_back(argv[i]);
        }

        emitlicenses(sourceFileNames, outfile);
    }

    return 0;
}
