#include "test/helpers/packages.h"

#include "ast/desugar/Desugar.h"
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

string PackageTextBuilder::build() {
    string out = fmt::format("class {} < PackageSpec\n", this->name);

    if (this->preludePackage) {
        fmt::format_to(back_inserter(out), "  prelude_package\n");
    }

    if (!this->strictDeps.empty()) {
        fmt::format_to(back_inserter(out), "  strict_dependencies '{}'\n", this->strictDeps);
    }

    if (!this->layer.empty()) {
        fmt::format_to(back_inserter(out), "  layer '{}'\n", this->layer);
    }

    for (auto &i : this->imports) {
        fmt::format_to(back_inserter(out), "  import {}\n", i);
    }

    for (auto &i : this->testImports) {
        fmt::format_to(back_inserter(out), "  test_import {}\n", i);
    }

    fmt::format_to(back_inserter(out), "end\n");

    return out;
}

string PackageHelpers::makePackageRB(string name, string strictDeps, string layer, vector<string> imports,
                                     vector<string> testImports) {
    return PackageTextBuilder()
        .withName(move(name))
        .withStrictDeps(move(strictDeps))
        .withLayer(move(layer))
        .withImports(move(imports))
        .withTestImports(move(testImports))
        .build();
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
        auto accessPackages = gs.unfreezePackages();
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
