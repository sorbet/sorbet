#include "doctest.h"

#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "common/concurrency/WorkerPool.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/GlobalSubstitution.h"
#include "core/Unfreeze.h"
#include "local_vars/local_vars.h"
#include "packager/packager.h"
#include "parser/parser.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

using namespace std;

namespace spd = spdlog;
auto logger = spd::stderr_color_mt("pkg-autocorrects-test");
auto errorQueue = make_shared<sorbet::core::ErrorQueue>(*logger, *logger);

namespace sorbet {
struct TestPackageFile {
    core::FileRef fileRef;
    ast::ParsedFile parsedFile;

    TestPackageFile(core::FileRef fileRef, ast::ParsedFile parsedFile)
        : fileRef(fileRef), parsedFile(move(parsedFile)) {}

    static TestPackageFile create(core::GlobalState &gs, string filename, string source) {
        // add the package file
        core::FileRef pkg_file;
        {
            core::UnfreezeFileTable fileTableAccess(gs);
            pkg_file = gs.enterFile(filename, source);
        }

        // run through the pipeline up through the packager
        // start by parsing and desugaring
        ast::ParsedFile parsed_file;
        {
            core::UnfreezeNameTable nameTableAccess(gs);
            // run parser
            auto nodes = parser::Parser::run(gs, pkg_file);
            // run desugarer
            core::MutableContext ctx(gs, core::Symbols::root(), pkg_file);
            parsed_file = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), pkg_file};

            // we can skip the rewriter because packages don't need it and
            // go straight on to indexing
            parsed_file = local_vars::LocalVars::run(ctx, move(parsed_file));
        }

        {
            // and then finally the packager!
            auto workers = WorkerPool::create(0, gs.tracer());
            vector<ast::ParsedFile> trees;
            trees.emplace_back(move(parsed_file));
            trees = packager::Packager::run(gs, *workers, move(trees));
            parsed_file = move(trees.front());
        }

        TestPackageFile pkgFile(pkg_file, move(parsed_file));

        return pkgFile;
    }
};

TEST_CASE("Simple add import") {
    sorbet::core::GlobalState gs(errorQueue);
    gs.initEmpty();

    auto test = TestPackageFile::create(gs, "my_package/__package.rb",
                                        "class Opus::MyPackage < PackageSpec\n  import Opus::SomethingeElse\nend\n");

    auto &package = gs.packageDB().getPackageForFile(gs, test.fileRef);
    ENFORCE(package.exists());
    gs.tracer().error("Got a package: {}", package.mangledName().show(gs));
    auto addImport = package.addImport(gs, package, false);
    if (addImport) {
        gs.tracer().error("Got an autocorrect: {}", addImport->edits.front().replacement);
    } else {
        gs.tracer().error("No autocorrect");
    }
    gs.tracer().error(":: {}", test.parsedFile.tree.toString(gs));

    CHECK_EQ("Yes", "Yes");
}
} // namespace sorbet
