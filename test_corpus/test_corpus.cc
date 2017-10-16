#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace spd = spdlog;

struct Expectations {
    std::string folder;
    std::string sourceFile;
    std::unordered_map<std::string, std::string> expactations;
};

std::vector<Expectations> inputs;

TEST(CorpusTest, PerPhaseTest) {
    auto console = spd::stdout_color_mt("fixtures");
    ruby_typer::ast::ContextBase ctx(*console);
    ruby_typer::ast::Context context(ctx, ctx.defn_root());

    for (auto &test : inputs) {
        auto inputPath = test.folder + test.sourceFile;

        if (test.expactations.find("parser") != test.expactations.end()) {
            auto checker = test.folder + test.expactations["parser"];
            SCOPED_TRACE(checker);

            auto src = ruby_typer::File::read(inputPath.c_str());
            auto exp = ruby_typer::File::read(checker.c_str());
            auto parsed = ruby_typer::parser::parse_ruby(ctx, src);

            EXPECT_EQ(0, parsed.diagnostics().size());
            EXPECT_EQ(exp, parsed.ast()->toString(ctx) + "\n");
            if (test.expactations.find("desugar") != test.expactations.end()) {
                auto checker = test.folder + test.expactations["desugar"];
                auto exp = ruby_typer::File::read(checker.c_str());
                SCOPED_TRACE(checker);

                auto desugared = ruby_typer::ast::desugar::node2Tree(context, parsed.ast());
                EXPECT_EQ(exp, desugared->toString(ctx) + "\n");
            }
        }
    }
}

bool endsWith(const std::string &a, const std::string &b) {
    if (b.size() > a.size())
        return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

static bool startsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.c_str(), prefix.size());
}

// substrantially modified from https://stackoverflow.com/a/8438663
void listDir(const char *name) {
    DIR *dir;
    struct dirent *entry;
    std::vector<std::string> names;

    if (!(dir = opendir(name))) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            listDir(path);
        } else {
            names.emplace_back(entry->d_name);
        }
    }
    std::sort(names.begin(), names.end());

    Expectations current;
    for (auto &s : names) {

        if (endsWith(s, ".rb")) {
            if (!current.sourceFile.empty()) {
                inputs.push_back(current);

                current.sourceFile.clear();
                current.expactations.clear();
            }
            current.sourceFile = s;
            current.folder = name;
            current.folder += "/";
        } else if (endsWith(s, ".exp")) {
            if (startsWith(s, current.sourceFile)) {
                auto kind_start = s.c_str() + current.sourceFile.size() + 1;
                auto kind_end = s.c_str() + s.size() - strlen(".exp");
                std::string kind(kind_start, kind_end - kind_start);
                current.expactations[kind] = s;
            }
        } else {
        }
    }
    if (!current.sourceFile.empty()) {
        inputs.push_back(current);

        current.sourceFile.clear();
        current.expactations.clear();
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    listDir("test_corpus/pos");
    for (auto &exp : inputs) {
        std::cout << exp.sourceFile << " {";
        for (auto &check : exp.expactations) {
            std::cout << check.first << " : " << check.second << ", ";
        }
        std::cout << "}" << std::endl;
    }
    return RUN_ALL_TESTS();
}