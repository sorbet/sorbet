#include "doctest.h"
// Include first as it uses poisoned things

#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "dtl/dtl.hpp"
#include "test/helpers/expectations.h"
#include <sstream>

using namespace std;

namespace sorbet::test {

namespace {
bool compareNames(string_view left, string_view right) {
    auto lsplit = left.find("__");
    if (lsplit == string::npos) {
        lsplit = left.find(".");
    }
    auto rsplit = right.find("__");
    if (rsplit == string::npos) {
        rsplit = right.find(".");
    }
    string_view lbase(left.data(), lsplit == string::npos ? left.size() : lsplit);
    string_view rbase(right.data(), rsplit == string::npos ? right.size() : rsplit);
    if (lbase != rbase) {
        return left < right;
    }

    // If the base names match, make files with the ".rb" extension come before all others.
    // The remaining files will be sorted by reverse order on extension.
    auto lext = FileOps::getExtension(left);
    auto rext = FileOps::getExtension(right);
    if (lext != rext) {
        if (lext == "rb") {
            return true;
        } else if (rext == "rb") {
            return false;
        } else {
            return rext < lext;
        }
    }

    // Sort multi-part tests
    return left < right;
}

string rbFile2BaseTestName(string rbFileName) {
    auto basename = rbFileName;
    auto lastDirSeparator = basename.find_last_of("/");
    if (lastDirSeparator != string::npos) {
        basename = basename.substr(lastDirSeparator + 1);
    }
    auto split = basename.rfind(".");
    if (split != string::npos) {
        basename = basename.substr(0, split);
    }
    split = basename.find("__");
    if (split != string::npos) {
        basename = basename.substr(0, split);
    }
    string testName = basename;
    if (lastDirSeparator != string::npos) {
        testName = rbFileName.substr(0, lastDirSeparator + 1 + testName.length());
    }
    return testName;
}

vector<Expectations> listDir(const char *name) {
    vector<Expectations> result;

    vector<string> names = sorbet::FileOps::listFilesInDir(name, {".rb", ".rbi", ".rbupdate", ".exp"}, false, {}, {});
    const int prefixLen = strnlen(name, 1024) + 1;
    // Trim off the input directory from the name.
    transform(names.begin(), names.end(), names.begin(),
              [&prefixLen](auto &name) -> string { return name.substr(prefixLen); });
    fast_sort(names, compareNames);

    Expectations current;
    for (auto &s : names) {
        if (absl::EndsWith(s, ".rb") || absl::EndsWith(s, ".rbi")) {
            auto basename = rbFile2BaseTestName(s);
            if (basename != s) {
                if (basename == current.basename) {
                    current.sourceFiles.emplace_back(s);
                    continue;
                }
            }

            if (!current.basename.empty()) {
                result.emplace_back(current);
                current = Expectations();
            }
            current.basename = basename;
            current.sourceFiles.emplace_back(s);
            current.folder = name;
            current.folder += "/";
            current.testName = current.folder + current.basename;
        } else if (absl::EndsWith(s, ".exp")) {
            if (absl::StartsWith(s, current.basename)) {
                auto kind_start = s.rfind(".", s.size() - strlen(".exp") - 1);
                string kind = s.substr(kind_start + 1, s.size() - kind_start - strlen(".exp") - 1);
                string source_file_path = string(name) + "/" + s.substr(0, kind_start);
                current.expectations[kind][source_file_path] = s;
            }
        } else if (absl::EndsWith(s, ".rbupdate")) {
            if (absl::StartsWith(s, current.basename)) {
                // Should be `.[number].rbupdate`
                auto pos = s.rfind('.', s.length() - 10);
                if (pos != string::npos) {
                    int version = stoi(s.substr(pos + 1, s.length() - 9));
                    current.sourceLSPFileUpdates[version].emplace_back(absl::StrCat(s.substr(0, pos), ".rb"), s);
                } else {
                    cout << "Ignoring " << s << ": No version number provided (expected .[number].rbupdate).\n";
                }
            }
        }
    }
    if (!current.basename.empty()) {
        result.emplace_back(current);
        current = Expectations();
    }

    return result;
}

} // namespace

Expectations Expectations::getExpectations(std::string singleTest) {
    vector<Expectations> result;
    if (singleTest.empty()) {
        Exception::raise("No test specified. Pass one with --single_test=<test_path>");
    }

    string parentDir;
    {
        auto lastDirSeparator = singleTest.find_last_of("/");
        if (lastDirSeparator == string::npos) {
            parentDir = ".";
        } else {
            parentDir = singleTest.substr(0, lastDirSeparator);
        }
    }
    auto scan = listDir(parentDir.c_str());
    auto lookingFor = rbFile2BaseTestName(singleTest);
    for (Expectations &f : scan) {
        if (f.testName == lookingFor) {
            for (auto &file : f.sourceFiles) {
                string filename = f.folder + file;
                string fileContents = FileOps::read(filename);
                f.sourceFileContents[filename] =
                    make_shared<core::File>(move(filename), move(fileContents), core::File::Type::Normal);
            }
            result.emplace_back(f);
        }
    }

    if (result.size() != 1) {
        Exception::raise("Expected exactly one test, found {}", result.size());
    }

    return result.front();
}

// A variant of CHECK_EQ that prints a diff on failure.
void CHECK_EQ_DIFF(std::string_view expected, std::string_view actual, std::string_view errorMessage) {
    if (expected == actual) {
        return;
    }

    vector<string> expectedLines = absl::StrSplit(expected, '\n');
    vector<string> actualLines = absl::StrSplit(actual, '\n');
    dtl::Diff<string, vector<string>> diff(expectedLines, actualLines);
    diff.compose();
    diff.composeUnifiedHunks();

    stringstream ss;
    diff.printUnifiedFormat(ss);
    FAIL_CHECK(fmt::format("{}\n{}", errorMessage, ss.str()));
}

} // namespace sorbet::test
