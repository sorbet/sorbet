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

bool addToExpectations(Expectations &exp, string_view parentDir, string_view filePath) {
    if (!absl::StartsWith(filePath, exp.basename)) {
        return false;
    }

    if (absl::EndsWith(filePath, ".rb") || absl::EndsWith(filePath, ".rbi")) {
        exp.sourceFiles.emplace_back(filePath);
        return true;
    } else if (absl::EndsWith(filePath, ".exp")) {
        auto kind_start = filePath.rfind(".", filePath.size() - strlen(".exp") - 1);
        auto kind = filePath.substr(kind_start + 1, filePath.size() - kind_start - strlen(".exp") - 1);
        string source_file_path = absl::StrCat(parentDir, "/", filePath.substr(0, kind_start));
        exp.expectations[kind][source_file_path] = filePath;
        return true;
    } else if (absl::EndsWith(filePath, ".rbupdate")) {
        // Should be `.[number].rbupdate`
        auto pos = filePath.rfind('.', filePath.length() - 10);
        if (pos != string::npos) {
            int version = stoi(string(filePath.substr(pos + 1, filePath.length() - 9)));
            exp.sourceLSPFileUpdates[version].emplace_back(absl::StrCat(filePath.substr(0, pos), ".rb"), filePath);
        } else {
            cout << "Ignoring " << filePath << ": No version number provided (expected .[number].rbupdate).\n";
        }
        return true;
    }

    return false;
}

vector<string> listTrimmedTestFilesInDir(string_view dir, bool recursive) {
    vector<string> names =
        sorbet::FileOps::listFilesInDir(dir, {".rb", ".rbi", ".rbupdate", ".exp"}, recursive, {}, {});
    const int prefixLen = dir.length() + 1;
    // Trim off the input directory from the name.
    transform(names.begin(), names.end(), names.begin(),
              [&prefixLen](auto &name) -> string { return name.substr(prefixLen); });
    fast_sort(names, compareNames);
    return names;
}

void populateSourceFileContents(Expectations &exp) {
    for (auto &file : exp.sourceFiles) {
        string filename = exp.folder + file;
        string fileContents = FileOps::read(filename);
        exp.sourceFileContents[filename] =
            make_shared<core::File>(move(filename), move(fileContents), core::File::Type::Normal);
    }
}

Expectations getExpectationsForFolderTest(string_view dir) {
    vector<string> names = listTrimmedTestFilesInDir(dir, true);
    ENFORCE(dir.back() == '/');

    Expectations exp;
    // No basename; all of these files belong to this expectations.
    exp.basename = "";
    exp.folder = dir;
    exp.testName = string(dir.substr(0, dir.length() - 1));

    for (auto &s : names) {
        addToExpectations(exp, dir, s);
    }

    populateSourceFileContents(exp);
    return exp;
}

vector<Expectations> listDir(string_view name) {
    vector<Expectations> result;
    vector<string> names = listTrimmedTestFilesInDir(name, false);

    Expectations current;
    for (auto &s : names) {
        if (current.basename.empty() || !addToExpectations(current, name, s)) {
            // `s` doesn't belong to current Expectations, _or_ Expectations is new.
            if (absl::EndsWith(s, ".rb") || absl::EndsWith(s, ".rbi")) {
                auto basename = rbFile2BaseTestName(s);
                if (!current.basename.empty()) {
                    // End of previously found test.
                    result.emplace_back(current);
                    current = Expectations();
                }
                // Start of new test.
                current.basename = basename;
                current.sourceFiles.emplace_back(s);
                current.folder = name;
                current.folder += "/";
                current.testName = current.folder + current.basename;
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

    if (FileOps::dirExists(singleTest)) {
        return getExpectationsForFolderTest(singleTest);
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
    auto scan = listDir(parentDir);
    auto lookingFor = rbFile2BaseTestName(singleTest);
    for (Expectations &f : scan) {
        if (f.testName == lookingFor) {
            populateSourceFileContents(f);
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
