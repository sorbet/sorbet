#include "test/helpers/packages.h"

#include "ast/desugar/Desugar.h"
#include "common/strings/formatting.h"
#include "core/FileRef.h"
#include "core/GlobalState.h"
#include "core/Unfreeze.h"
#include "local_vars/local_vars.h"
#include "main/options/options.h"
#include "main/pipeline/pipeline.h"
#include "namer/namer.h"
#include "packager/packager.h"
#include "parser/parser.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::test {

const vector<string> PackageHelpers::NO_LAYERS = {};
const vector<string> PackageHelpers::LAYERS_LIB_APP = {"lib", "app"};
const vector<string> PackageHelpers::LAYERS_UTIL_LIB_APP = {"util", "lib", "app"};

string PackageHelpers::makePackageRB(string name, string strictDeps, string layer, vector<string> imports,
                                     vector<string> testImports) {
    auto importList =
        fmt::map_join(imports, "", [&](const auto &import) { return fmt::format("  import {}\n", import); });

    auto testImportList = fmt::map_join(
        testImports, "", [&](const auto &testImport) { return fmt::format("  test_import {}\n", testImport); });

    return fmt::format("class {} < PackageSpec\n"
                       "  strict_dependencies '{}'\n"
                       "  layer '{}'\n"
                       "{}"
                       "{}"
                       "end",
                       name, strictDeps, layer, importList, testImportList);
}

void PackageHelpers::makeDefaultPackagerGlobalState(core::GlobalState &gs, const vector<string> &packagerLayers) {
    gs.initEmpty();
    realmain::options::Options opts;
    opts.cacheSensitiveOptions.sorbetPackages = true;
    opts.packagerLayers = packagerLayers;
    realmain::pipeline::setGlobalStateOptions(gs, opts);
}

vector<ast::ParsedFile> PackageHelpers::enterPackages(core::GlobalState &gs,
                                                      vector<pair<string, string>> packageSources) {
    vector<core::FileRef> files;
    {
        core::UnfreezeFileTable fileTableAccess(gs);
        for (auto &[filename, source] : packageSources) {
            files.emplace_back(gs.enterFile(filename, source));
        }
    }

    // run through the pipeline up through the packager
    // start by parsing and desugaring
    vector<ast::ParsedFile> parsedFiles;
    {
        core::UnfreezeNameTable nameTableAccess(gs);
        // run parser
        for (auto file : files) {
            auto settings = parser::Parser::Settings{};
            auto nodes = parser::Parser::run(gs, file, settings).tree;

            core::MutableContext ctx(gs, core::Symbols::root(), file);
            auto parsedFile = ast::ParsedFile{ast::desugar::node2Tree(ctx, move(nodes)), file};
            parsedFile.tree = rewriter::Rewriter::run(ctx, move(parsedFile.tree));
            parsedFiles.emplace_back(local_vars::LocalVars::run(ctx, move(parsedFile)));
        }
    }

    auto workers = WorkerPool::create(0, gs.tracer());

    {
        core::UnfreezeNameTable nameTableAccess(gs);
        core::UnfreezeSymbolTable symbolTableAccess(gs);
        auto foundHashes = nullptr;
        auto canceled = namer::Namer::run(gs, absl::MakeSpan(parsedFiles), *workers, foundHashes);
        ENFORCE(!canceled);
    }

    packager::Packager::run(gs, *workers, absl::Span<ast::ParsedFile>(parsedFiles));
    return parsedFiles;
}

const core::packages::PackageInfo &PackageHelpers::packageInfoFor(const core::GlobalState &gs, core::FileRef file) {
    return gs.packageDB().getPackageInfo(gs.packageDB().getPackageNameForFile(file));
}

} // namespace sorbet::test
