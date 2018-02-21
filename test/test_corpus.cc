#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/common.h"
#include "core/ErrorQueue.h"
#include "core/Unfreeze.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "payload/binary/binary.h"
#include "resolver/resolver.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cstdio>
#include <cxxopts.hpp>
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
    string basename;
    vector<string> sourceFiles;
    unordered_map<string, string> expectations;
};

int shardId;
int totalShards;

vector<Expectations> getInputs(int myId, int totalShards);

string prettyPrintTest(testing::TestParamInfo<Expectations> arg) {
    string res = arg.param.folder + arg.param.basename;
    auto ext = ruby_typer::File::getExtension(res);
    if (ext == "rb") {
        res.erase(res.end() - ext.size() - 1, res.end());
    }
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
    bool raw = false;
    bool typedSource = false;

public:
    CFG_Collector_and_Typer(bool raw = false, bool typedSource = false) : raw(raw), typedSource(typedSource){};
    vector<string> cfgs;
    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::core::Context ctx, ruby_typer::ast::MethodDef *m) {
        auto cfg = ruby_typer::cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);
        if (raw || typedSource) {
            cfg = ruby_typer::cfg::CFGBuilder::addDebugEnvironment(ctx.withOwner(m->symbol), move(cfg));
        }
        cfg = ruby_typer::infer::Inference::run(ctx.withOwner(m->symbol), move(cfg));
        if (typedSource) {
            cfg->recordAnnotations(ctx);
        }

        cfgs.push_back(cfg->toString(ctx));
        return m;
    }
};

class ErrorAndPos {
public:
    string error;
    int beginPos = -1;
    int endPos = -1;
};

unordered_set<string> knownPasses = {"parse-tree",   "parse-tree-json", "ast",         "ast-raw",       "dsl-tree",
                                     "dsl-tree-raw", "name-table",      "name-tree",   "name-tree-raw", "cfg",
                                     "cfg-raw",      "infer",           "typed-source"};

TEST(CorpusTest, CloneSubstitutePayload) {
    auto logger = spd::stderr_color_mt("ClonePayload");
    auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger);

    ruby_typer::core::GlobalState gs(errorQueue);
    ruby_typer::core::serialize::GlobalStateSerializer::load(gs, getNameTablePayload);

    auto c1 = gs.deepCopy();
    auto c2 = gs.deepCopy();

    ruby_typer::core::NameRef n1;
    {
        ruby_typer::core::UnfreezeNameTable thaw1(*c1);
        n1 = c1->enterNameUTF8("test new name");
    }

    ruby_typer::core::GlobalSubstitution subst(*c1, *c2);
    ASSERT_EQ("test new name", subst.substitute(n1).toString(*c2));
    ASSERT_EQ(c1->symbolsUsed(), c2->symbolsUsed());
    ASSERT_EQ(c1->symbolsUsed(), gs.symbolsUsed());
}

TEST_P(ExpectationTest, PerPhaseTest) { // NOLINT
    vector<unique_ptr<ruby_typer::core::BasicError>> errors;
    Expectations test = GetParam();
    auto inputPath = test.folder + test.basename;
    SCOPED_TRACE(inputPath);

    for (auto &exp : test.expectations) {
        auto it = knownPasses.find(exp.first);
        if (it == knownPasses.end()) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    auto logger = spd::stderr_color_mt("fixtures: " + inputPath);
    auto errorQueue = std::make_shared<ruby_typer::core::ErrorQueue>(*logger);
    ruby_typer::core::GlobalState gs(errorQueue);
    ruby_typer::core::serialize::GlobalStateSerializer::load(gs, getNameTablePayload);
    ruby_typer::core::MutableContext ctx(gs, ruby_typer::core::Symbols::root());

    // Parser
    vector<ruby_typer::core::FileRef> files;
    {
        ruby_typer::core::UnfreezeFileTable fileTableAccess(gs);

        for (auto &srcPath : test.sourceFiles) {
            auto path = test.folder + srcPath;
            auto src = ruby_typer::File::read(path.c_str());
            files.push_back(gs.enterFile(path, src));
        }
    }
    vector<unique_ptr<ruby_typer::ast::Expression>> trees;
    map<string, string> got;

    for (auto file : files) {
        std::unique_ptr<ruby_typer::parser::Node> nodes;
        {
            ruby_typer::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            nodes = ruby_typer::parser::Parser::run(gs, file);
        }
        {
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        auto expectation = test.expectations.find("parse-tree");
        if (expectation != test.expectations.end()) {
            got["parse-tree"].append(nodes->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("parse-tree-json");
        if (expectation != test.expectations.end()) {
            got["parse-tree-json"].append(nodes->toJSON(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        // Desugarer
        std::unique_ptr<ruby_typer::ast::Expression> desugared;
        {
            ruby_typer::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            desugared = ruby_typer::ast::desugar::node2Tree(ctx, move(nodes));
        }

        expectation = test.expectations.find("ast");
        if (expectation != test.expectations.end()) {
            got["ast"].append(desugared->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("ast-raw");
        if (expectation != test.expectations.end()) {
            got["ast-raw"].append(desugared->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        // DSL
        std::unique_ptr<ruby_typer::ast::Expression> dslUnwound;
        {
            ruby_typer::core::UnfreezeNameTable nameTableAccess(gs); // enters original strings

            dslUnwound = ruby_typer::dsl::DSL::run(ctx, move(desugared));
        }

        expectation = test.expectations.find("dsl-tree");
        if (expectation != test.expectations.end()) {
            got["dsl-tree"].append(dslUnwound->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("dsl-tree-raw");
        if (expectation != test.expectations.end()) {
            got["dsl-tree-raw"].append(dslUnwound->showRaw(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        // Namer
        std::unique_ptr<ruby_typer::ast::Expression> namedTree;
        {
            ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
            ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
            namedTree = ruby_typer::namer::Namer::run(ctx, move(dslUnwound));
        }
        trees.emplace_back(move(namedTree));
    }

    {
        ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
        ruby_typer::core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
        trees = ruby_typer::resolver::Resolver::run(ctx, move(trees));
    }

    auto expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        got["name-table"] = gs.toString() + "\n";
        auto newErrors = errorQueue->drainErrors();
        errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                      std::make_move_iterator(newErrors.end()));
    }

    for (auto &resolvedTree : trees) {
        expectation = test.expectations.find("name-tree");
        if (expectation != test.expectations.end()) {
            got["name-tree"].append(resolvedTree->toString(gs)).append("\n");
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("name-tree-raw");
        if (expectation != test.expectations.end()) {
            got["name-tree-raw"].append(resolvedTree->showRaw(gs));
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }
    }

    for (auto &resolvedTree : trees) {
        auto file = resolvedTree->loc.file;
        auto checkTree = [&]() {
            if (resolvedTree == nullptr) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1) << "Already used tree. You can only have 1 CFG-ish .exp file";
            }
        };
        auto checkPragma = [&](string ext) {
            if (file.data(ctx).source_type != ruby_typer::core::File::Typed) {
                auto path = file.data(ctx).path();
                ADD_FAILURE_AT(path.begin(), 1)
                    << "Missing `@typed` pragma. Sources with ." << ext << ".exp expectations must specify @typed.";
            }
        };
        // CFG

        expectation = test.expectations.find("cfg");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("cfg");
            CFG_Collector_and_Typer collector;
            auto cfg = ruby_typer::ast::TreeMap<CFG_Collector_and_Typer, ruby_typer::core::Context>::apply(
                ctx, collector, move(resolvedTree));

            stringstream dot;
            dot << "digraph \"" << ruby_typer::File::getFileName(inputPath) << "\"{" << endl;
            for (auto &cfg : collector.cfgs) {
                dot << cfg << endl << endl;
            }
            dot << "}" << endl << endl;
            got["cfg"].append(dot.str());

            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("cfg-raw");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("cfg-raw");
            CFG_Collector_and_Typer collector(true);
            auto cfg = ruby_typer::ast::TreeMap<CFG_Collector_and_Typer, ruby_typer::core::Context>::apply(
                ctx, collector, move(resolvedTree));

            stringstream dot;
            dot << "digraph \"" << ruby_typer::File::getFileName(inputPath) << "\"{" << endl;
            for (auto &cfg : collector.cfgs) {
                dot << cfg << endl << endl;
            }
            dot << "}" << endl << endl;
            got["cfg-raw"].append(dot.str());

            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("typed-source");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("typed-source");
            CFG_Collector_and_Typer collector(false, true);
            ruby_typer::ast::TreeMap<CFG_Collector_and_Typer, ruby_typer::core::Context>::apply(ctx, collector,
                                                                                                move(resolvedTree));

            got["typed-source"].append(gs.showAnnotatedSource(file));

            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }

        expectation = test.expectations.find("infer");
        if (expectation != test.expectations.end()) {
            checkTree();
            checkPragma("infer");
            CFG_Collector_and_Typer collector;
            ruby_typer::ast::TreeMap<CFG_Collector_and_Typer, ruby_typer::core::Context>::apply(ctx, collector,
                                                                                                move(resolvedTree));
            auto checker = test.folder + expectation->second;
            SCOPED_TRACE(checker);

            got["infer"] = "";
            auto newErrors = errorQueue->drainErrors();
            errors.insert(errors.end(), std::make_move_iterator(newErrors.begin()),
                          std::make_move_iterator(newErrors.end()));
        }
    }

    for (auto &gotPhase : got) {
        auto expectation = test.expectations.find(gotPhase.first);
        ASSERT_TRUE(expectation != test.expectations.end()) << "missing expectation for " << gotPhase.first;
        auto checker = test.folder + expectation->second;
        auto expect = ruby_typer::File::read(checker.c_str());
        EXPECT_EQ(expect, gotPhase.second) << "Mismatch on: " << checker;
        if (expect == gotPhase.second) {
            TEST_COUT << gotPhase.first << " ok" << endl;
        }
    }

    expectation = test.expectations.find("name-table");
    if (expectation != test.expectations.end()) {
        string table = gs.toString() + "\n";
        EXPECT_EQ(got["name-table"], table) << " name-table should not be mutated by CFG+inference";
    }

    // Check warnings and errors

    map<pair<ruby_typer::core::FileRef, int>, ErrorAndPos> expectedErrors;
    regex errorRegex("# error: ?(.*)");           // something like 'foo   # error: Badness'
    regex errorPosRegex("([ ]*#[ ]+)(\\^+)[ ]*"); // someting like '   #    ^^^^^  '
    regex commendOut("^[ ]*#");

    for (auto file : files) {
        string src(file.data(gs).source().begin(), file.data(gs).source().end());
        stringstream ss(src);
        string line;
        int linenum = 1;

        while (getline(ss, line, '\n')) {
            smatch matches;
            if (regex_search(line, matches, errorRegex)) {
                if (regex_search(line, commendOut)) {
                    continue;
                }
                string match = matches[1].str();
                int len = match.size();
                if (len < 10 && match.find("MULTI") == string::npos) {
                    ADD_FAILURE_AT(file.data(gs).path().data(), linenum)
                        << "Too short of a error message at " << len
                        << " characters. Use MULTI or write something longer than: " << match;
                }
                expectedErrors[make_pair(file, linenum)].error = match;
            } else if (regex_match(line, matches, errorPosRegex)) {
                auto expectedError = expectedErrors.find(make_pair(file, linenum - 1));
                if (expectedError == expectedErrors.end()) {
                    ADD_FAILURE_AT(file.data(gs).path().data(), linenum)
                        << "Position comment must come right after a line with a `error:` comment. Found position "
                           "comment on line "
                        << linenum << " matching " << matches[0].str();
                }
                expectedError->second.beginPos = matches[1].str().size() + 1; // We start our columns at 1
                expectedError->second.endPos = expectedError->second.beginPos + matches[2].str().size();
            }
            linenum += 1;
        }
    }

    map<pair<ruby_typer::core::FileRef, int>, int> seenErrorLines;
    int unknownLocErrorLine = 1;
    for (auto &error : errors) {
        auto filePath = error->loc.file.data(gs).path();
        if (error->loc.is_none()) {
            // The convention is to put `error: Unknown Location Error` at
            // the top of the file for each of these so that they are eaten
            // first when reporting mismatched errors.
            int line = unknownLocErrorLine++;
            auto expectedError = expectedErrors.find(make_pair(files.front(), line));
            if (expectedError == expectedErrors.end()) {
                ADD_FAILURE_AT(filePath.data(), line) << "Unknown location error thrown but not annotated." << endl
                                                      << "Reported error: " << error->formatted;
            } else if (error->formatted.find(expectedError->second.error) == string::npos) {
                ADD_FAILURE_AT(filePath.data(), line) << "Error string mismatch." << endl
                                                      << " Expectation: " << expectedError->second.error << endl
                                                      << " Reported error: " << error->formatted;
            } else {
                seenErrorLines[make_pair(files.front(), line)]++;
            }
            continue;
        }

        auto pos = error->loc.position(gs);
        bool found = false;
        for (int i = pos.first.line; i <= pos.second.line; i++) {
            auto expectedErrorIt = expectedErrors.find(make_pair(error->loc.file, i));
            if (expectedErrorIt != expectedErrors.end()) {
                string expectedError = expectedErrorIt->second.error;
                found = true;
                seenErrorLines[make_pair(error->loc.file, i)]++;
                if (expectedError.empty()) {
                    ADD_FAILURE_AT(filePath.data(), i) << "Error occurred, but no expected text found. Please put (a "
                                                          "substring of) the expected error after `# error:` "
                                                       << endl
                                                       << "The message was: '" << error->formatted << "'";
                } else if (expectedError.find("MULTI") != string::npos) { // multiple errors. Ignore message and pos
                    continue;
                } else if (error->formatted.find(expectedError) == string::npos) {
                    ADD_FAILURE_AT(filePath.data(), i) << "Error string mismatch." << endl
                                                       << " Expectation: " << expectedError << endl
                                                       << " Reported error: " << error->formatted;
                } else if (expectedErrorIt->second.beginPos != -1 &&
                           pos.first.column != expectedErrorIt->second.beginPos) {
                    ADD_FAILURE_AT(filePath.data(), i)
                        << "Wrong starting error position. Expected error starting at position "
                        << expectedErrorIt->second.beginPos << " but found one starting at position "
                        << pos.first.column;
                } else if (expectedErrorIt->second.endPos != -1 &&
                           pos.second.column != expectedErrorIt->second.endPos) {
                    ADD_FAILURE_AT(filePath.data(), i)
                        << "Wrong ending error position. Expected error ending at position "
                        << expectedErrorIt->second.endPos << " but found one ending at position " << pos.second.column;
                }
            }
        }
        if (!found) {
            ADD_FAILURE_AT(filePath.data(), pos.first.line) << "Unexpected error:\n " << error->toString(gs);
        }
    }

    for (auto &error : expectedErrors) {
        auto filePath = error.first.first.data(gs).path();
        if (seenErrorLines.find(error.first) == seenErrorLines.end()) {
            ADD_FAILURE_AT(filePath.data(), error.first.second) << "Expected error did not happen.";
        }
        if (error.second.error.find("MULTI") != string::npos && seenErrorLines[error.first] == 1) {
            ADD_FAILURE_AT(filePath.data(), error.first.second) << "Expected multiple errors, but only saw one.";
        }
    }

    // Allow later phases to have errors that we didn't test for
    errorQueue->drainErrors();

    TEST_COUT << "errors OK" << endl;
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, testing::ValuesIn(getInputs(shardId, totalShards)), prettyPrintTest);

bool endsWith(const string &a, const string &b) {
    if (b.size() > a.size()) {
        return false;
    }
    return equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

static bool startsWith(const string &str, const string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

bool compareNames(const string &left, const string &right) {
    auto lsplit = left.find("__");
    if (lsplit == string::npos) {
        lsplit = left.find(".");
    }
    auto rsplit = right.find("__");
    if (rsplit == string::npos) {
        rsplit = right.find(".");
    }
    absl::string_view lbase(left.data(), lsplit == string::npos ? left.size() : lsplit);
    absl::string_view rbase(right.data(), rsplit == string::npos ? right.size() : rsplit);
    if (lbase != rbase) {
        return left < right;
    }

    // If the base names match, compare by reverse order on extension, so that
    // .exp comes after .rb.
    auto lext = ruby_typer::File::getExtension(left);
    auto rext = ruby_typer::File::getExtension(right);
    if (lext != rext) {
        return rext < lext;
    }

    // Sort multi-part tests
    return left < right;
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
            if (strcmp(entry->d_name, "disabled") == 0) {
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
    sort(names.begin(), names.end(), compareNames);

    Expectations current;
    for (auto &s : names) {
        if (endsWith(s, ".rb")) {
            auto basename = s;
            auto split = s.find("__");
            if (split != string::npos) {
                basename = s.substr(0, split);
                if (basename == current.basename) {
                    current.sourceFiles.push_back(s);
                    continue;
                }
            }

            if (!current.basename.empty()) {
                result.push_back(current);
                current = Expectations();
            }
            current.basename = basename;
            current.sourceFiles.push_back(s);
            current.folder = name;
            current.folder += "/";
        } else if (endsWith(s, ".exp")) {
            if (startsWith(s, current.basename)) {
                auto kind_start = s.c_str() + current.basename.size() + 1;
                auto kind_end = s.c_str() + s.size() - strlen(".exp");
                string kind(kind_start, kind_end - kind_start);
                current.expectations[kind] = s;
            }
        } else {
        }
    }
    if (!current.basename.empty()) {
        result.push_back(current);
        current = Expectations();
    }

    closedir(dir);
    return result;
}

vector<Expectations> getInputs(int myId, int totalShards) {
    auto all = listDir("test/testdata");
    vector<Expectations> result;
    for (Expectations &f : all) {
        if (((std::hash<string>()(f.basename) % totalShards) + totalShards) % totalShards == myId) {
            result.push_back(f);
        }
    }
    return result;
}

int main(int argc, char **argv) {
    cxxopts::Options options("test_corpus", "Test corpus for Ruby Typer");
    options.add_options()("shards_total", "Number of parallel test workers", cxxopts::value<int>()->default_value("1"),
                          "shards");
    options.add_options()("my_id", "ID of this worker across parallel shards. Should be in [0..shards_total)",
                          cxxopts::value<int>()->default_value("0"), "id");
    options.parse(argc, argv);
    shardId = options["my_id"].as<int>();
    totalShards = options["shards_total"].as<int>();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
