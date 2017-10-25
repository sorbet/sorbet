#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "cfg/CFG.h"
#include "common/common.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <memory>
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
    virtual ~ExpectationTest() {}
    virtual void SetUp() {
        exp_ = GetParam();
    }
    virtual void TearDown() {}

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
    ~TestCout() {
        PRINTF("%s", str().c_str());
    }
};

#define TEST_COUT TestCout()

class CFG_Collector {
public:
    vector<string> cfgs;
    ruby_typer::ast::MethodDef *preTransformMethodDef(ruby_typer::ast::Context ctx, ruby_typer::ast::MethodDef *m) {
        cfgs.push_back(ruby_typer::cfg::CFG::buildFor(ctx, *m)->toString(ctx));
        return m;
    }
};

unordered_set<string> knownPasses = {
    "parse-tree",
    "ast",
    "name-table",
    "cfg",
};

TEST_P(ExpectationTest, PerPhaseTest) {
    Expectations test = GetParam();
    auto inputPath = test.folder + test.sourceFile;
    SCOPED_TRACE(inputPath);

    auto console = spd::stdout_color_mt("fixtures: " + inputPath);
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::ast::Context context(ctx, ctx.defn_root());

    auto src = ruby_typer::File::read(inputPath.c_str());
    auto parsed = ruby_typer::parser::parse_ruby(ctx, inputPath, src);

    for (auto &exp : test.expectations) {
        auto it = knownPasses.find(exp.first);
        if (it == knownPasses.end()) {
            ADD_FAILURE() << "Unknown pass: " << exp.first;
        }
    }

    expectation = test.expectations.find("name-tree");
    if (expectation == test.expectations.end())
        return;

    auto namedTree = ruby_typer::namer::Namer::run(context, std::move(desugared));
    {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, namedTree->toString(ctx) + "\n");
        if (exp == namedTree->toString(ctx) + "\n") {
            TEST_COUT << "name-tree OK" << std::endl;
        }
    }

    expectation = test.expectations.find("name-table");
    if (expectation == test.expectations.end())
        return;

    {
        auto checker = test.folder + expectation->second;
        auto exp = ruby_typer::File::read(checker.c_str());
        SCOPED_TRACE(checker);

        EXPECT_EQ(exp, ctx.toString() + "\n");
        if (exp == ctx.toString() + "\n") {
            TEST_COUT << "name-table OK" << std::endl;
        }
    }

    expectation = test.expectations.find("cfg");
    if (expectation == test.expectations.end())
        return;

    CFG_Collector collector;
    auto cfg = ruby_typer::ast::TreeMap<CFG_Collector>::apply(context, collector, move(namedTree));

    {
        auto checker = test.folder + expectation->second;
        SCOPED_TRACE(checker);

        auto exp = ruby_typer::File::read(checker.c_str());

        stringstream got;
        for (auto &cfg : collector.cfgs) {
            got << cfg << endl << endl;
        }
        EXPECT_EQ(exp, got.str() + "\n");
        if (exp == got.str() + "\n") {
            TEST_COUT << "cfg OK" << endl;
        }
    }
}

INSTANTIATE_TEST_CASE_P(PosTests, ExpectationTest, testing::ValuesIn(getInputs()), prettyPrintTest);

bool endsWith(const string &a, const string &b) {
    if (b.size() > a.size())
        return false;
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

    if (!(dir = opendir(name))) {
        return result;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
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
