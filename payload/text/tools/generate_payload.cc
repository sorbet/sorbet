#include "common/FileOps.h"
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
    absl::c_replace(sourceName, '.', '_');
    absl::c_replace(sourceName, '/', '_');
    absl::c_replace(sourceName, '-', '_');
    return sourceName;
}

string sorbetVersion(string defaultVersion) {
    string version;
    if (sorbet::Version::isReleaseBuild) {
        version = sorbet::Version::build_scm_revision;
    } else {
        version = defaultVersion;
    }
    return version;
}

string filePath2Github(string file, string branch) {
    const char prefix[] = "rbi/sorbet-typed/lib/";
    if (file.rfind(prefix, 0) != 0) {
        return "https://github.com/stripe/sorbet/tree/" + branch + "/" + file;
    }
    file = file.substr(std::strlen(prefix));
    return "https://github.com/sorbet/sorbet-typed/tree/" + branch + "/lib/" + file;
}

void emit_classfile(vector<string> sourceFiles, ostream &out) {
    string version = sorbetVersion("master");

    out << "#include<string_view>" << '\n' << "#include<vector>\nusing namespace std;\n";
    out << "namespace sorbet{" << '\n' << "namespace rbi{" << '\n';
    for (auto &file : sourceFiles) {
        out << "  string_view " + sourceName2funcName(file) << "() {" << '\n';
        out << "  return \"" + absl::CEscape(sorbet::FileOps::read(file.c_str())) + "\"sv;" << '\n' << "}" << '\n';
    }
    out << "vector<pair<string_view, string_view> > all() {" << '\n';
    out << "  vector<pair<string_view, string_view> > result;" << '\n';
    for (auto &file : sourceFiles) {
        string permalink = filePath2Github(file, version);
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
