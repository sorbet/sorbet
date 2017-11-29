#include "../namer/namer.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/common.h"
#include "core/serialize/serialize.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace spd = spdlog;
using namespace std;

struct Expectations {
    string folder;
    string sourceFile;
    unordered_map<string, string> expectations;
};

vector<Expectations> getInputs();

string prettyPrintTest(testing::TestParamInfo<Expectations> arg) {
    string res = arg.param.folder + arg.param.sourceFile;
    res.erase(res.size() - strlen(".rb"), strlen(".rb"));
    replace(res.begin(), res.end(), '/', '_');
    return res;
}

class ExpectationTest : public testing::TestWithParam<Expectations> {
public:
    ~ExpectationTest() override = default;
    void SetUp() override {
        exp_ = GetParam();
    }
    void TearDown() override {}

protected:
    Expectations exp_;
};

// taken from https://stackoverflow.com/questions/16491675/how-to-send-custom-message-in-google-c-testing-framework
namespace testing {
namespace internal {
enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };

extern void ColoredPrintf(GTestColor color, const char *fmt, ...);
} // namespace internal
} // namespace testing

#define PRINTF(...)                                                                        \
    do {                                                                                   \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
        testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, __VA_ARGS__);     \
    } while (0)

// C++ stream interface
class TestCout : public stringstream {
public:
    ~TestCout() override {
        PRINTF("%s", str().c_str());
    }
};

#define TEST_COUT TestCout()

class CFG_Collector_and_Typer {
public:
    CFG_Collector_and_Typer() = default;
    vector<string> cfgs;
    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::core::Context ctx, ruby_typer::ast::MethodDef *m) {
        auto cfg = ruby_typer::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        ruby_typer::infer::Inference::run(ctx.withOwner(m->symbol), cfg);

        cfgs.push_back(cfg->toString(ctx));
        return m;
    }
};

unordered_set<string> knownPasses = {
    "parse-tree", "ast", "ast-raw", "name-table", "name-tree", "name-tree-raw", "cfg", "infer",
};

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<ruby_typer::core::Reporter::BasicError>> errors;
    Expectations test = GetParam();
    auto inputPath = test.folder + test.sourceFile;
    SCOPED_TRACE(inputPath);

    for (auto &exp : test.expectations) {
        auto it = knownPasses.find(exp.first);
        if (it == knownPasses.end()) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    auto console = spd::stderr_color_mt("fixtures: " + inputPath);
    ruby_typer::core::GlobalState gs =
        ruby_typer::core::serialize::GlobalStateSerializer::load(getNameTablePayload, *console);
    gs.freshNameId = 10000;
    ruby_typer::core::Context context(gs, gs.defn_root());
    gs.errors.keepErrorsInMemory = true;

    // Parser
    auto src = ruby_typer::File::read(inputPath.c_str());
    auto ast = ruby_typer::parser::Parser::run(gs, inputPath, src);
    {
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    auto expectation = test.expectations.find("parse-tree");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        SCOPED_TRACE(checker);

        auto exp = ruby_typer::File::read(checker.c_str());

        EXPECT_EQ(exp, ast->toString(gs) + "\n");
        if (exp == ast->toString(gs) + "\n") {
            TEST_COUT << "parse-tree OK" << endl;
        }
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    // Desugarer
    auto desugared = ruby_typer::ast::desugar::node2Tree(context, ast);

    expectation = test.expectations.find("ast");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, desugared->toString(gs) + "\n");
        if (exp == desugared->toString(gs) + "\n") {
            TEST_COUT << "ast OK" << endl;
        }
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("ast-raw");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, desugared->showRaw(gs) + "\n");
        if (exp == desugared->showRaw(gs) + "\n") {
            TEST_COUT << "ast-raw OK" << endl;
        }
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    // Namer
    auto namedTree = ruby_typer::namer::Namer::run(context, move(desugared));
    auto resolvedTree = ruby_typer::namer::Resolver::run(context, move(namedTree));
    ruby_typer::namer::Resolver::finalize(context);

    expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("name-tree");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, resolvedTree->toString(gs) + "\n");
        if (exp == resolvedTree->toString(gs) + "\n") {
            TEST_COUT << "name-tree OK" << endl;
        }
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("name-tree-raw");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, resolvedTree->showRaw(gs) + "\n");
        if (exp == resolvedTree->showRaw(gs) + "\n") {
            TEST_COUT << "name-tree-raw OK" << endl;
        }
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    // CFG
    CFG_Collector_and_Typer collector;
    auto cfg = ruby_typer::ast::TreeMap<CFG_Collector_and_Typer>::apply(context, collector, move(resolvedTree));

    expectation = test.expectations.find("cfg");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        SCOPED_TRACE(checker);

        auto exp = ruby_typer::File::read(checker.c_str());

        stringstream got;
        got << "digraph \"" + ruby_typer::File::getFileName(inputPath) + "\"{" << endl;
        for (auto &cfg : collector.cfgs) {
            got << cfg << endl << endl;
        }
        got << "}" << endl;
        EXPECT_EQ(exp, got.str() + "\n");
        if (exp == got.str() + "\n") {
            TEST_COUT << "cfg+infer OK" << endl;
        }

        ifstream svgFile(checker + ".svg");
        EXPECT_TRUE(svgFile.good());
        auto newErrors = gs.errors.getAndEmptyErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, gs.toString() + "\n");
        if (exp == gs.toString() + "\n") {
            TEST_COUT << "name-table OK" << endl;
        }
    }

    // Check warnings and errors

    map<int, string> expectedErrors;
    string line;
    stringstream ss(src);
    int linenum = 1;
    regex errorRegex("# error: ?(.*)");

    while (getline(ss, line, '\n')) {
        smatch matches;
        if (regex_search(line, matches, errorRegex)) {
            string match = matches[1].str();
            int len = match.size();
            if (len < 10 && match.find("MULTI") == string::npos) {
                ADD_FAILURE_AT(inputPath.c_str(), linenum)
                    << "Too short of a error message at " << len
                    << " characters. Use MULTI or write something longer than: " << match;
            }
            expectedErrors[linenum] = match;
        }
        linenum += 1;
    }

    map<int, int> seenErrorLines;
    int unknownLocErrorLine = 1;
    for (auto &error : errors) {
        if (error->loc.is_none()) {
            // The convention is to put `error: Unknown Location Error` at
            // the top of the file for each of these so that they are eaten
            // first when reporting mismatched errors.
            int line = unknownLocErrorLine++;
            auto expectedError = expectedErrors.find(line);
            if (expectedError == expectedErrors.end()) {
                ADD_FAILURE_AT(inputPath.c_str(), line) << "Unknown location error thrown but not annotated.";
            } else if (error->formatted.find(expectedError->second) == string::npos) {
                ADD_FAILURE_AT(inputPath.c_str(), line) << "Error string mismatch." << endl
                                                        << " Expectation: " << expectedError->second << endl
                                                        << " Reported error: " << error->formatted;
            } else {
                seenErrorLines[line]++;
            }
            continue;
        }

        auto pos = error->loc.position(gs);
        bool found = false;
        for (int i = pos.first.line; i <= pos.second.line; i++) {
            auto expectedError = expectedErrors.find(i);
            if (expectedError != expectedErrors.end()) {
                bool isMultipleErrors =
                    expectedError->second.find("MULTI") != string::npos; // multiple errors. Ignore message
                if (expectedError->second.empty()) {
                    ADD_FAILURE_AT(inputPath.c_str(), i) << "Error occurred, but no expected text found. Please put (a "
                                                            "substring of) the expected error after `# error:` "
                                                         << endl
                                                         << "The message was: '" << error->formatted << "'";
                } else if (error->formatted.find(expectedError->second) == string::npos && !isMultipleErrors) {
                    ADD_FAILURE_AT(inputPath.c_str(), i) << "Error string mismatch." << endl
                                                         << " Expectation: " << expectedError->second << endl
                                                         << " Reported error: " << error->formatted;
                } else {
                    found = true;
                    seenErrorLines[i]++;
                    continue;
                }
            }
        }
        if (!found) {
            ADD_FAILURE_AT(inputPath.c_str(), pos.first.line) << "Unexpected error:\n " << error->toString(gs);
        }
    }

    for (auto &error : expectedErrors) {
        if (seenErrorLines.find(error.first) == seenErrorLines.end()) {
            ADD_FAILURE_AT(inputPath.c_str(), error.first) << "Expected error did not happen.";
        }
        if (error.second.find("MULTI") != string::npos && seenErrorLines[error.first] == 1) {
            ADD_FAILURE_AT(inputPath.c_str(), error.first) << "Expected multiple errors, but only saw one.";
        }
    }

    TEST_COUT << "errors OK" << endl;
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, testing::ValuesIn(getInputs()), prettyPrintTest);

bool endsWith(const string &a, const string &b) {
    if (b.size() > a.size()) {
        return false;
    }
    return equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

static bool startsWith(const string &str, const string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

// substrantially modified from https://stackoverflow.com/a/8438663
vector<Expectations> listDir(const char *name) {
    vector<Expectations> result;
    DIR *dir;
    struct dirent *entry;
    vector<string> names;

    if ((dir = opendir(name)) == nullptr) {
        return result;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            auto nested = listDir(path);
            result.insert(result.end(), nested.begin(), nested.end());
        } else {
            names.emplace_back(entry->d_name);
        }
    }
    sort(names.begin(), names.end());

    Expectations current;
    for (auto &s : names) {
        if (endsWith(s, ".rb")) {
            if (!current.sourceFile.empty()) {
                result.push_back(current);

                current.sourceFile.clear();
                current.expectations.clear();
            }
            current.sourceFile = s;
            current.folder = name;
            current.folder += "/";
        } else if (endsWith(s, ".exp")) {
            if (startsWith(s, current.sourceFile)) {
                auto kind_start = s.c_str() + current.sourceFile.size() + 1;
                auto kind_end = s.c_str() + s.size() - strlen(".exp");
                string kind(kind_start, kind_end - kind_start);
                current.expectations[kind] = s;
            }
        } else {
        }
    }
    if (!current.sourceFile.empty()) {
        result.push_back(current);

        current.sourceFile.clear();
        current.expectations.clear();
    }

    closedir(dir);
    return result;
}

vector<Expectations> getInputs() {
    return listDir("test/testdata");
}
