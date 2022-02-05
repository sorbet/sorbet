#include "common/FileOps.h"
#include "common/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/escaping.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

string sourceName2funcName(string sourceName) {
    absl::c_replace(sourceName, '.', '_');
    absl::c_replace(sourceName, '/', '_');
    absl::c_replace(sourceName, '-', '_');
    return sourceName;
}

string addSourceURLToTypedLine(string originalSource, string url) {
    auto start = originalSource.find("typed:", 0);
    if (start == string_view::npos) {
        return originalSource;
    }
    auto endOfLine = originalSource.find("\n", start);
    originalSource.insert(endOfLine, " source is at " + url);
    return originalSource;
}

void emit_classfile(vector<string> sourceFiles, ostream &out) {
    out << "#include<string_view>" << '\n' << "#include<vector>\nusing namespace std;\n";
    out << "namespace sorbet{" << '\n' << "namespace rbi{" << '\n';
    string version;
    if (sorbet_is_release_build) {
        version = sorbet_build_scm_revision;
    } else {
        version = "master";
    }
    for (auto &file : sourceFiles) {
        string permalink = "https://github.com/sorbet/sorbet/tree/" + version + "/" + file;
        out << "  string_view " + sourceName2funcName(file) << "() {" << '\n';
        size_t nullPadding = 2;
        out << "  return \"" +
                   absl::CEscape(addSourceURLToTypedLine(sorbet::FileOps::read(file.c_str(), nullPadding), permalink)) +
                   "\"sv;"
            << '\n'
            << "}" << '\n';
    }
    out << "vector<pair<string_view, string_view> > all() {" << '\n';
    out << "  vector<pair<string_view, string_view> > result;" << '\n';
    for (auto &file : sourceFiles) {
        string permalink = "https://github.com/sorbet/sorbet/tree/" + version + "/" + file;
        out << "  result.emplace_back(make_pair<string_view, string_view>(\"" + absl::CEscape(permalink) + "\"sv, " +
                   sourceName2funcName(file) + "()));"
            << '\n';
    }
    out << "  return result;" << '\n';

    out << "}}};" << '\n';
}

int main(int argc, char **argv) {
    // emit header file
    {
        ofstream classfile(argv[1], ios::trunc);
        if (!classfile.good()) {
            cerr << "unable to open " << argv[2] << '\n';
            return 1;
        }

        vector<string> sources;
        for (int i = 2; i < argc; i++) {
            sources.emplace_back(argv[i]);
        }

        emit_classfile(sources, classfile);
    }

    return 0;
}
