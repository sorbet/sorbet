#ifdef SORBET_REALMAIN_MIN
// minimal build to speedup compilation. Remove extra features
#else
// has to go first, as it violates poisons
#include "core/proto/proto.h"
#include "namer/configatron/configatron.h"
#include "plugin/Plugins.h"
#include "plugin/SubprocessTextPlugin.h"
#include <sstream>
#endif
#include "ProgressIndicator.h"
#include "absl/strings/escaping.h" // BytesToHexString
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "class_flatten/class_flatten.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/ErrorQueue.h"
#include "core/GlobalSubstitution.h"
#include "core/NameHash.h"
#include "core/Unfreeze.h"
#include "core/errors/parser.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "core/serialize/serialize.h"
#include "definition_validator/validator.h"
#include "infer/infer.h"
#include "local_vars/local_vars.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "pipeline.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::realmain::pipeline {

class CFGCollectorAndTyper {
    const options::Options &opts;

public:
    CFGCollectorAndTyper(const options::Options &opts) : opts(opts){};

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->loc.file().data(ctx).strictLevel < core::StrictLevel::True || m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);

        if (opts.stopAfterPhase == options::Phase::CFG) {
            return m;
        }
        cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
        if (cfg) {
            for (auto &extension : ctx.state.semanticExtensions) {
                extension->typecheck(ctx, *cfg, m);
            }
        }
        if (print.CFG.enabled) {
            print.CFG.fmt("{}\n\n", cfg->toString(ctx));
        }
        if (print.CFGRaw.enabled) {
            print.CFGRaw.fmt("{}\n\n", cfg->showRaw(ctx));
        }
        return m;
    }
};

string fileKey(const core::File &file) {
    auto path = file.path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = sorbet::crypto_hashing::hash64(file.source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

unique_ptr<core::serialize::CachedFile> fetchFileFromCache(core::GlobalState &gs, core::FileRef fref,
                                                           const core::File &file,
                                                           const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (kvstore && fref.id() < gs.filesUsed()) {
        string fileHashKey = fileKey(file);
        auto maybeCached = kvstore->read(fileHashKey);
        if (maybeCached) {
            prodCounterInc("types.input.files.kvstore.hit");
            auto cachedTree = core::serialize::Serializer::loadFile(gs, fref, maybeCached);
            ENFORCE(cachedTree.tree->loc.file() == fref);
            return make_unique<core::serialize::CachedFile>(move(cachedTree));
        } else {
            prodCounterInc("types.input.files.kvstore.miss");
        }
    }
    return nullptr;
}

unique_ptr<ast::Expression> fetchTreeFromCache(core::GlobalState &gs, core::FileRef fref, const core::File &file,
                                               const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    auto cachedFile = fetchFileFromCache(gs, fref, file, kvstore);
    if (cachedFile) {
        return move(cachedFile->tree);
    }
    return nullptr;
}

unique_ptr<parser::Node> runParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print) {
    Timer timeit(gs.tracer(), "runParser", {{"file", (string)file.data(gs).path()}});
    unique_ptr<parser::Node> nodes;
    {
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        nodes = parser::Parser::run(gs, file);
    }
    if (print.ParseTree.enabled) {
        print.ParseTree.fmt("{}\n", nodes->toStringWithTabs(gs, 0));
    }
    if (print.ParseTreeJson.enabled) {
        print.ParseTreeJson.fmt("{}\n", nodes->toJSON(gs, 0));
    }
    if (print.ParseTreeWhitequark.enabled) {
        print.ParseTreeWhitequark.fmt("{}\n", nodes->toWhitequark(gs, 0));
    }
    return nodes;
}

unique_ptr<ast::Expression> runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                                       const options::Printers &print) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", (string)file.data(gs).path()}});
    unique_ptr<ast::Expression> ast;
    core::MutableContext ctx(gs, core::Symbols::root());
    {
        core::ErrorRegion errs(gs, file);
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(ctx, move(parseTree));
    }
    if (print.DesugarTree.enabled) {
        print.DesugarTree.fmt("{}\n", ast->toStringWithTabs(gs, 0));
    }
    if (print.DesugarTreeRaw.enabled) {
        print.DesugarTreeRaw.fmt("{}\n", ast->showRaw(gs));
    }
    return ast;
}

unique_ptr<ast::Expression> runRewriter(core::GlobalState &gs, core::FileRef file, unique_ptr<ast::Expression> ast) {
    core::MutableContext ctx(gs, core::Symbols::root());
    Timer timeit(gs.tracer(), "runRewriter", {{"file", (string)file.data(gs).path()}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
    core::ErrorRegion errs(gs, file);
    return rewriter::Rewriter::run(ctx, move(ast));
}

ast::ParsedFile runLocalVars(core::GlobalState &gs, ast::ParsedFile tree) {
    Timer timeit(gs.tracer(), "runLocalVars", {{"file", (string)tree.file.data(gs).path()}});
    core::MutableContext ctx(gs, core::Symbols::root());
    return sorbet::local_vars::LocalVars::run(ctx, move(tree));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {ast::MK::EmptyTree(), file};
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         unique_ptr<ast::Expression> tree) {
    auto &print = opts.print;
    ast::ParsedFile rewriten{nullptr, file};
    ENFORCE(file.data(lgs).strictLevel == decideStrictLevel(lgs, file, opts));

    Timer timeit(lgs.tracer(), "indexOne");
    try {
        if (!tree) {
            // tree isn't cached. Need to start from parser
            if (file.data(lgs).strictLevel == core::StrictLevel::Ignore) {
                return emptyParsedFile(file);
            }
            auto parseTree = runParser(lgs, file, print);
            if (opts.stopAfterPhase == options::Phase::PARSER) {
                return emptyParsedFile(file);
            }
            tree = runDesugar(lgs, file, move(parseTree), print);
            if (opts.stopAfterPhase == options::Phase::DESUGARER) {
                return emptyParsedFile(file);
            }
            if (!opts.skipRewriterPasses) {
                tree = runRewriter(lgs, file, move(tree));
            }
            tree = runLocalVars(lgs, ast::ParsedFile{move(tree), file}).tree;
            if (opts.stopAfterPhase == options::Phase::LOCAL_VARS) {
                return emptyParsedFile(file);
            }
        }
        if (print.RewriterTree.enabled) {
            print.RewriterTree.fmt("{}\n", tree->toStringWithTabs(lgs, 0));
        }
        if (print.RewriterTreeRaw.enabled) {
            print.RewriterTreeRaw.fmt("{}\n", tree->showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyParsedFile(file);
        }

        rewriten.tree = move(tree);
        ENFORCE(rewriten.tree->loc.file() == file);
        return rewriten;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = lgs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(lgs).path());
        }
        return emptyParsedFile(file);
    }
}

pair<ast::ParsedFile, vector<shared_ptr<core::File>>> emptyPluginFile(core::FileRef file) {
    return {emptyParsedFile(file), vector<shared_ptr<core::File>>()};
}

pair<ast::ParsedFile, vector<shared_ptr<core::File>>> indexOneWithPlugins(const options::Options &opts,
                                                                          core::GlobalState &gs, core::FileRef file,
                                                                          unique_ptr<ast::Expression> tree) {
    auto &print = opts.print;
    ast::ParsedFile rewriten{nullptr, file};
    vector<shared_ptr<core::File>> resultPluginFiles;

    Timer timeit(gs.tracer(), "indexOneWithPlugins", {{"file", (string)file.data(gs).path()}});
    try {
        if (!tree) {
            // tree isn't cached. Need to start from parser
            if (file.data(gs).strictLevel == core::StrictLevel::Ignore) {
                return emptyPluginFile(file);
            }
            auto parseTree = runParser(gs, file, print);
            if (opts.stopAfterPhase == options::Phase::PARSER) {
                return emptyPluginFile(file);
            }
            tree = runDesugar(gs, file, move(parseTree), print);
            if (opts.stopAfterPhase == options::Phase::DESUGARER) {
                return emptyPluginFile(file);
            }

#ifndef SORBET_REALMAIN_MIN
            {
                Timer timeit(gs.tracer(), "plugins_text");
                core::MutableContext ctx(gs, core::Symbols::root());
                core::ErrorRegion errs(gs, file);

                auto [pluginTree, pluginFiles] = plugin::SubprocessTextPlugin::run(ctx, move(tree));
                tree = move(pluginTree);
                resultPluginFiles = move(pluginFiles);
            }
#endif
            if (!opts.skipRewriterPasses) {
                tree = runRewriter(gs, file, move(tree));
            }
            if (print.RewriterTree.enabled) {
                print.RewriterTree.fmt("{}\n", tree->toStringWithTabs(gs, 0));
            }
            if (print.RewriterTreeRaw.enabled) {
                print.RewriterTreeRaw.fmt("{}\n", tree->showRaw(gs));
            }

            tree = runLocalVars(gs, ast::ParsedFile{move(tree), file}).tree;
            if (opts.stopAfterPhase == options::Phase::LOCAL_VARS) {
                return emptyPluginFile(file);
            }
        }
        if (print.IndexTree.enabled) {
            print.IndexTree.fmt("{}\n", tree->toStringWithTabs(gs, 0));
        }
        if (print.IndexTreeRaw.enabled) {
            print.IndexTreeRaw.fmt("{}\n", tree->showRaw(gs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyPluginFile(file);
        }

        rewriten.tree = move(tree);
        ENFORCE(rewriten.tree->loc.file() == file);
        return {move(rewriten), resultPluginFiles};
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(gs).path());
        }
        return emptyPluginFile(file);
    }
}

vector<ast::ParsedFile> incrementalResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                                           const options::Options &opts) {
    try {
        core::MutableContext ctx(gs, core::Symbols::root());
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            what = sorbet::namer::Namer::run(ctx, move(what));
        }

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");
            core::ErrorRegion errs(gs, sorbet::core::FileRef());
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            what = sorbet::resolver::Resolver::runTreePasses(ctx, move(what));
        }
    } catch (SorbetException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

vector<core::FileRef> reserveFiles(unique_ptr<core::GlobalState> &gs, const vector<string> &files) {
    Timer timeit(gs->tracer(), "reserveFiles");
    vector<core::FileRef> ret;
    core::UnfreezeFileTable unfreezeFiles(*gs);
    for (auto f : files) {
        auto fileRef = gs->findFileByPath(f);
        if (!fileRef.exists()) {
            fileRef = gs->reserveFileRef(f);
        }
        ret.emplace_back(move(fileRef));
    }
    return ret;
}

core::StrictLevel decideStrictLevel(const core::GlobalState &gs, const core::FileRef file,
                                    const options::Options &opts) {
    auto &fileData = file.data(gs);

    core::StrictLevel level;
    string filePath = string(fileData.path());
    // make sure all relative file paths start with ./
    if (!absl::StartsWith(filePath, "/") && !absl::StartsWith(filePath, "./")) {
        filePath.insert(0, "./");
    }

    if (fileData.originalSigil == core::StrictLevel::None) {
        level = core::StrictLevel::False;
    } else {
        level = fileData.originalSigil;
    }

    core::StrictLevel minStrict = opts.forceMinStrict;
    core::StrictLevel maxStrict = opts.forceMaxStrict;
    if (level <= core::StrictLevel::Max && level > core::StrictLevel::Ignore) {
        level = max(min(level, maxStrict), minStrict);
    }

    auto fnd = opts.strictnessOverrides.find(filePath);
    if (fnd != opts.strictnessOverrides.end()) {
        if (fnd->second == fileData.originalSigil && fnd->second > opts.forceMinStrict &&
            fnd->second < opts.forceMaxStrict) {
            core::ErrorRegion errs(gs, file);
            if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Parser::ParserError)) {
                e.setHeader("Useless override of strictness level");
            }
        }
        level = fnd->second;
    }

    if (gs.runningUnderAutogen) {
        // Autogen stops before infer but needs to see all definitions
        level = core::StrictLevel::False;
    }

    return level;
}

void incrementStrictLevelCounter(core::StrictLevel level) {
    switch (level) {
        case core::StrictLevel::None:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::Ignore:
            prodCounterInc("types.input.files.sigil.ignore");
            break;
        case core::StrictLevel::Internal:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::False:
            prodCounterInc("types.input.files.sigil.false");
            break;
        case core::StrictLevel::True:
            prodCounterInc("types.input.files.sigil.true");
            break;
        case core::StrictLevel::Strict:
            prodCounterInc("types.input.files.sigil.strict");
            break;
        case core::StrictLevel::Strong:
            prodCounterInc("types.input.files.sigil.strong");
            break;
        case core::StrictLevel::Max:
            Exception::raise("Should never happen");
            break;
        case core::StrictLevel::Autogenerated:
            prodCounterInc("types.input.files.sigil.autogenerated");
            break;
        case core::StrictLevel::Stdlib:
            prodCounterInc("types.input.files.sigil.stdlib");
            break;
    }
}

// Returns a non-null ast::Expression if kvstore contains the AST.
unique_ptr<ast::Expression> readFileWithStrictnessOverrides(unique_ptr<core::GlobalState> &gs, core::FileRef file,
                                                            const options::Options &opts,
                                                            const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    unique_ptr<ast::Expression> ast;
    if (file.dataAllowingUnsafe(*gs).sourceType != core::File::Type::NotYetRead) {
        return ast;
    }
    auto fileName = file.dataAllowingUnsafe(*gs).path();
    Timer timeit(gs->tracer(), "readFileWithStrictnessOverrides", {{"file", (string)fileName}});
    string src;
    bool fileFound = true;
    try {
        src = opts.fs->readFile(fileName);
    } catch (FileNotFoundException e) {
        // continue with an empty source, because the
        // assertion below requires every input file to map
        // to one output tree
        fileFound = false;
    }
    prodCounterAdd("types.input.bytes", src.size());
    prodCounterInc("types.input.files");

    {
        core::UnfreezeFileTable unfreezeFiles(*gs);
        auto fileObj =
            make_shared<core::File>(string(fileName.begin(), fileName.end()), move(src), core::File::Type::Normal);
        auto maybeCached = fetchFileFromCache(*gs, file, *fileObj, kvstore);
        if (maybeCached) {
            fileObj = move(maybeCached->file);
            ast = move(maybeCached->tree);
        }

        auto entered = gs->enterNewFileAt(move(fileObj), file);
        ENFORCE(entered == file);
    }
    if (enable_counters) {
        counterAdd("types.input.lines", file.data(*gs).lineCount());
    }

    auto &fileData = file.data(*gs);
    if (!fileFound) {
        if (auto e = gs->beginError(sorbet::core::Loc::none(file), core::errors::Internal::FileNotFound)) {
            e.setHeader("File Not Found");
        }
    }

    if (!opts.storeState.empty()) {
        fileData.sourceType = core::File::Type::PayloadGeneration;
    }

    auto level = decideStrictLevel(*gs, file, opts);
    fileData.strictLevel = level;
    incrementStrictLevelCounter(level);
    return ast;
}

struct IndexResult {
    unique_ptr<core::GlobalState> gs;
    vector<ast::ParsedFile> trees;
    vector<shared_ptr<core::File>> pluginGeneratedFiles;
};

struct IndexThreadResultPack {
    CounterState counters;
    IndexResult res;
};

IndexResult mergeIndexResults(const shared_ptr<core::GlobalState> cgs, const options::Options &opts,
                              shared_ptr<BlockingBoundedQueue<IndexThreadResultPack>> input,
                              const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    ProgressIndicator progress(opts.showProgress, "Indexing", input->bound);
    Timer timeit(cgs->tracer(), "mergeIndexResults");
    IndexThreadResultPack threadResult;
    IndexResult ret;
    for (auto result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs->tracer()); !result.done();
         result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs->tracer())) {
        if (result.gotItem()) {
            counterConsume(move(threadResult.counters));
            if (ret.gs == nullptr) {
                ret.gs = move(threadResult.res.gs);
                ENFORCE(ret.trees.empty());
                ret.trees = move(threadResult.res.trees);
                ret.pluginGeneratedFiles = move(threadResult.res.pluginGeneratedFiles);
            } else {
                core::GlobalSubstitution substitution(*threadResult.res.gs, *ret.gs, cgs.get());
                core::MutableContext ctx(*ret.gs, core::Symbols::root());
                {
                    Timer timeit(cgs->tracer(), "substituteTrees");
                    for (auto &tree : threadResult.res.trees) {
                        auto file = tree.file;
                        core::ErrorRegion errs(*ret.gs, file);
                        if (!file.data(*ret.gs).cached) {
                            tree.tree = ast::Substitute::run(ctx, substitution, move(tree.tree));
                        }
                    }
                }
                ret.trees.insert(ret.trees.end(), make_move_iterator(threadResult.res.trees.begin()),
                                 make_move_iterator(threadResult.res.trees.end()));

                ret.pluginGeneratedFiles.insert(ret.pluginGeneratedFiles.end(),
                                                make_move_iterator(threadResult.res.pluginGeneratedFiles.begin()),
                                                make_move_iterator(threadResult.res.pluginGeneratedFiles.end()));
            }
            progress.reportProgress(input->doneEstimate());
            ret.gs->errorQueue->flushErrors();
        }
    }
    return ret;
}

IndexResult indexSuppliedFiles(const shared_ptr<core::GlobalState> &baseGs, vector<core::FileRef> &files,
                               const options::Options &opts, WorkerPool &workers,
                               const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    Timer timeit(baseGs->tracer(), "indexSuppliedFiles");
    auto resultq = make_shared<BlockingBoundedQueue<IndexThreadResultPack>>(files.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(files.size());
    for (auto &file : files) {
        fileq->push(move(file), 1);
    }

    workers.multiplexJob("indexSuppliedFiles", [baseGs, &opts, fileq, resultq, &kvstore]() {
        Timer timeit(baseGs->tracer(), "indexSuppliedFilesWorker");
        unique_ptr<core::GlobalState> localGs = baseGs->deepCopy();
        IndexThreadResultPack threadResult;

        {
            core::FileRef job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    core::FileRef file = job;
                    auto cachedTree = readFileWithStrictnessOverrides(localGs, file, opts, kvstore);
                    auto [parsedFile, pluginFiles] = indexOneWithPlugins(opts, *localGs, file, move(cachedTree));
                    threadResult.res.pluginGeneratedFiles.insert(threadResult.res.pluginGeneratedFiles.end(),
                                                                 make_move_iterator(pluginFiles.begin()),
                                                                 make_move_iterator(pluginFiles.end()));
                    threadResult.res.trees.emplace_back(move(parsedFile));
                }
            }
        }

        if (!threadResult.res.trees.empty()) {
            threadResult.counters = getAndClearThreadCounters();
            threadResult.res.gs = move(localGs);
            auto computedTreesCount = threadResult.res.trees.size();
            resultq->push(move(threadResult), computedTreesCount);
        }
    });

    return mergeIndexResults(baseGs, opts, resultq, kvstore);
}

IndexResult indexPluginFiles(IndexResult firstPass, const options::Options &opts, WorkerPool &workers,
                             const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (firstPass.pluginGeneratedFiles.empty()) {
        return firstPass;
    }
    Timer timeit(firstPass.gs->tracer(), "indexPluginFiles");
    auto resultq = make_shared<BlockingBoundedQueue<IndexThreadResultPack>>(firstPass.pluginGeneratedFiles.size());
    auto pluginFileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(firstPass.pluginGeneratedFiles.size());
    {
        core::UnfreezeFileTable unfreezeFiles(*firstPass.gs);
        for (const auto &file : firstPass.pluginGeneratedFiles) {
            auto generatedFile = firstPass.gs->enterFile(file);
            pluginFileq->push(move(generatedFile), 1);
        }
    }
    const shared_ptr<core::GlobalState> protoGs = move(firstPass.gs);
    workers.multiplexJob("indexPluginFiles", [protoGs, &opts, pluginFileq, resultq, &kvstore]() {
        Timer timeit(protoGs->tracer(), "indexPluginFilesWorker");
        auto localGs = protoGs->deepCopy();
        IndexThreadResultPack threadResult;
        core::FileRef job;

        for (auto result = pluginFileq->try_pop(job); !result.done(); result = pluginFileq->try_pop(job)) {
            if (result.gotItem()) {
                core::FileRef file = job;
                file.data(*localGs).strictLevel = decideStrictLevel(*localGs, file, opts);
                threadResult.res.trees.emplace_back(
                    indexOne(opts, *localGs, file, fetchTreeFromCache(*localGs, file, file.data(*localGs), kvstore)));
            }
        }

        if (!threadResult.res.trees.empty()) {
            threadResult.counters = getAndClearThreadCounters();
            threadResult.res.gs = move(localGs);
            auto sizeIncrement = threadResult.res.trees.size();
            resultq->push(move(threadResult), sizeIncrement);
        }
    });
    auto indexedPluginFiles = mergeIndexResults(protoGs, opts, resultq, kvstore);
    IndexResult suppliedFilesAndPluginFiles;
    if (indexedPluginFiles.trees.empty()) {
        return firstPass;
    }
    suppliedFilesAndPluginFiles.gs = move(indexedPluginFiles.gs);

    {
        Timer timeit(suppliedFilesAndPluginFiles.gs->tracer(), "incremental_resolve");
        core::GlobalSubstitution substitution(*protoGs, *suppliedFilesAndPluginFiles.gs, protoGs.get());
        core::MutableContext ctx(*suppliedFilesAndPluginFiles.gs, core::Symbols::root());
        for (auto &tree : firstPass.trees) {
            auto file = tree.file;
            core::ErrorRegion errs(*suppliedFilesAndPluginFiles.gs, file);
            tree.tree = ast::Substitute::run(ctx, substitution, move(tree.tree));
        }
    }
    suppliedFilesAndPluginFiles.trees = move(firstPass.trees);
    suppliedFilesAndPluginFiles.trees.insert(suppliedFilesAndPluginFiles.trees.end(),
                                             make_move_iterator(indexedPluginFiles.trees.begin()),
                                             make_move_iterator(indexedPluginFiles.trees.end()));
    return suppliedFilesAndPluginFiles;
}

vector<ast::ParsedFile> index(unique_ptr<core::GlobalState> &gs, vector<core::FileRef> files,
                              const options::Options &opts, WorkerPool &workers,
                              const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    Timer timeit(gs->tracer(), "index");
    vector<ast::ParsedFile> ret;
    vector<ast::ParsedFile> empty;

    if (opts.stopAfterPhase == options::Phase::INIT) {
        return empty;
    }

    gs->sanityCheck();

    if (files.size() < 3) {
        // Run singlethreaded if only using 2 files
        size_t pluginFileCount = 0;
        for (auto file : files) {
            auto tree = readFileWithStrictnessOverrides(gs, file, opts, kvstore);
            auto [parsedFile, pluginFiles] = indexOneWithPlugins(opts, *gs, file, move(tree));
            ret.emplace_back(move(parsedFile));
            pluginFileCount += pluginFiles.size();
            for (auto &pluginFile : pluginFiles) {
                core::FileRef pluginFileRef;
                {
                    core::UnfreezeFileTable fileTableAccess(*gs);
                    pluginFileRef = gs->enterFile(pluginFile);
                    pluginFileRef.data(*gs).strictLevel = decideStrictLevel(*gs, pluginFileRef, opts);
                }
                ret.emplace_back(
                    indexOne(opts, *gs, pluginFileRef, fetchTreeFromCache(*gs, pluginFileRef, *pluginFile, kvstore)));
            }
        }
        ENFORCE(files.size() + pluginFileCount == ret.size());
    } else {
        auto firstPass = indexSuppliedFiles(move(gs), files, opts, workers, kvstore);
        auto pluginPass = indexPluginFiles(move(firstPass), opts, workers, kvstore);
        gs = move(pluginPass.gs);
        ret = move(pluginPass.trees);
    }

    fast_sort(ret, [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
    return ret;
}

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts) {
    ast::ParsedFile result{ast::MK::EmptyTree(), resolved.file};
    core::FileRef f = resolved.file;

    resolved = definition_validator::runOne(ctx, std::move(resolved));

    resolved = class_flatten::runOne(ctx, move(resolved));

    if (opts.print.FlattenTree.enabled || opts.print.AST.enabled) {
        opts.print.FlattenTree.fmt("{}\n", resolved.tree->toString(ctx));
    }
    if (opts.print.FlattenTreeRaw.enabled || opts.print.ASTRaw.enabled) {
        opts.print.FlattenTreeRaw.fmt("{}\n", resolved.tree->showRaw(ctx));
    }

    if (opts.stopAfterPhase == options::Phase::NAMER || opts.stopAfterPhase == options::Phase::RESOLVER) {
        return result;
    }
    if (f.data(ctx).isRBI()) {
        return result;
    }

    Timer timeit(ctx.state.tracer(), "typecheckOne", {{"file", (string)f.data(ctx).path()}});
    try {
        if (opts.print.CFG.enabled) {
            opts.print.CFG.fmt("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
        }
        if (opts.print.CFGRaw.enabled) {
            opts.print.CFGRaw.fmt("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
            opts.print.CFGRaw.fmt("  graph [fontname = \"Courier\"];\n");
            opts.print.CFGRaw.fmt("  node [fontname = \"Courier\"];\n");
            opts.print.CFGRaw.fmt("  edge [fontname = \"Courier\"];\n");
        }
        CFGCollectorAndTyper collector(opts);
        {
            core::ErrorRegion errs(ctx, f);
            result.tree = ast::TreeMap::apply(ctx, collector, move(resolved.tree));
            for (auto &extension : ctx.state.semanticExtensions) {
                extension->finishTypecheckFile(ctx, f);
            }
        }
        if (opts.print.CFG.enabled) {
            opts.print.CFG.fmt("}}\n\n");
        }
        if (opts.print.CFGRaw.enabled) {
            opts.print.CFGRaw.fmt("}}\n\n");
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = ctx.state.beginError(sorbet::core::Loc::none(f), core::errors::Internal::InternalError)) {
            e.setHeader("Exception in cfg+infer: {} (backtrace is above)", f.data(ctx).path());
        }
    }
    return result;
}

struct typecheck_thread_result {
    vector<ast::ParsedFile> trees;
    CounterState counters;
};

vector<ast::ParsedFile> name(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                             bool skipConfigatron) {
    Timer timeit(gs.tracer(), "name");
    if (!skipConfigatron) {
#ifndef SORBET_REALMAIN_MIN
        core::UnfreezeNameTable nameTableAccess(gs);     // creates names from config
        core::UnfreezeSymbolTable symbolTableAccess(gs); // creates methods for them
        namer::configatron::fillInFromFileSystem(gs, opts.configatronDirs, opts.configatronFiles);
#endif
    }

    {
        core::MutableContext ctx(gs, core::Symbols::root());
        core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
        core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
        what = namer::Namer::run(ctx, move(what));
        gs.errorQueue->flushErrors();
    }
    return what;
}
class GatherUnresolvedConstantsWalk {
public:
    vector<string> unresolvedConstants;
    unique_ptr<ast::Expression> postTransformConstantLit(core::MutableContext ctx,
                                                         unique_ptr<ast::ConstantLit> original) {
        auto unresolvedPath = original->fullUnresolvedPath(ctx);
        if (unresolvedPath.has_value()) {
            unresolvedConstants.emplace_back(fmt::format(
                "{}::{}",
                unresolvedPath->first != core::Symbols::root() ? unresolvedPath->first.data(ctx)->show(ctx) : "",
                fmt::map_join(unresolvedPath->second,
                              "::", [&](const auto &el) -> string { return el.data(ctx)->show(ctx); })));
        }
        return original;
    }
};

vector<ast::ParsedFile> printMissingConstants(core::GlobalState &gs, const options::Options &opts,
                                              vector<ast::ParsedFile> what) {
    Timer timeit(gs.tracer(), "printMissingConstants");
    core::MutableContext ctx(gs, core::Symbols::root());
    GatherUnresolvedConstantsWalk walk;
    for (auto &resolved : what) {
        resolved.tree = ast::TreeMap::apply(ctx, walk, move(resolved.tree));
    }
    auto &missing = walk.unresolvedConstants;
    fast_sort(missing);
    missing.erase(unique(missing.begin(), missing.end()), missing.end());

    opts.print.MissingConstants.fmt("{}\n", fmt::join(missing, "\n"));
    return what;
}

class DefinitionLinesBlacklistEnforcer {
private:
    const core::FileRef file;
    const int prohibitedLinesStart;
    const int prohibitedLinesEnd;

    bool isWhiteListed(core::Context ctx, core::SymbolRef sym) {
        return sym.data(ctx)->name == core::Names::staticInit() ||
               sym.data(ctx)->name == core::Names::Constants::Root() ||
               sym.data(ctx)->name == core::Names::unresolvedAncestors();
    }

    void checkLoc(core::Context ctx, core::Loc loc) {
        auto detailStart = core::Loc::offset2Pos(file.data(ctx), loc.beginPos());
        auto detailEnd = core::Loc::offset2Pos(file.data(ctx), loc.endPos());
        ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
    }

    void checkSym(core::Context ctx, core::SymbolRef sym) {
        if (isWhiteListed(ctx, sym)) {
            return;
        }
        checkLoc(ctx, sym.data(ctx)->loc());
    }

public:
    DefinitionLinesBlacklistEnforcer(core::FileRef file, int prohibitedLinesStart, int prohibitedLinesEnd)
        : file(file), prohibitedLinesStart(prohibitedLinesStart), prohibitedLinesEnd(prohibitedLinesEnd) {
        // Can be equal if file was empty.
        ENFORCE(prohibitedLinesStart <= prohibitedLinesEnd);
        ENFORCE(file.exists());
    };

    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> original) {
        checkSym(ctx, original->symbol);
        return original;
    }
    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> original) {
        checkSym(ctx, original->symbol);
        return original;
    }
};

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd) {
    DefinitionLinesBlacklistEnforcer enforcer(what.file, prohibitedLinesStart, prohibitedLinesEnd);
    what.tree = ast::TreeMap::apply(core::Context(gs, core::Symbols::root()), enforcer, move(what.tree));
    return what;
}

ast::ParsedFilesOrCancelled resolve(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers, bool skipConfigatron) {
    try {
        what = name(*gs, move(what), opts, skipConfigatron);
        if (gs->epochManager->wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled();
        }

        for (auto &named : what) {
            if (opts.print.NameTree.enabled) {
                opts.print.NameTree.fmt("{}\n", named.tree->toStringWithTabs(*gs, 0));
            }
            if (opts.print.NameTreeRaw.enabled) {
                opts.print.NameTreeRaw.fmt("{}\n", named.tree->showRaw(*gs));
            }
        }

        if (opts.stopAfterPhase == options::Phase::NAMER) {
            return ast::ParsedFilesOrCancelled(move(what));
        }

        core::MutableContext ctx(*gs, core::Symbols::root());
        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(gs->tracer(), "resolving");
            vector<core::ErrorRegion> errs;
            for (auto &tree : what) {
                auto file = tree.file;
                errs.emplace_back(*gs, file);
            }
            core::UnfreezeNameTable nameTableAccess(*gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters stubs
            auto maybeResult = resolver::Resolver::run(ctx, move(what), workers);
            if (!maybeResult.hasResult()) {
                return maybeResult;
            }
            what = move(maybeResult.result());
        }
        if (opts.stressIncrementalResolver) {
            auto symbolsBefore = gs->symbolsUsed();
            for (auto &f : what) {
                // Shift contents of file past current file's EOF, re-run incrementalResolve, assert that no locations
                // appear before file's old EOF.
                const int prohibitedLines = f.file.data(*gs).source().size();
                auto newSource = fmt::format("{}\n{}", string(prohibitedLines, '\n'), f.file.data(*gs).source());
                auto newFile = make_shared<core::File>((string)f.file.data(*gs).path(), move(newSource),
                                                       f.file.data(*gs).sourceType);
                gs = core::GlobalState::replaceFile(move(gs), f.file, move(newFile));
                f.file.data(*gs).strictLevel = decideStrictLevel(*gs, f.file, opts);
                auto reIndexed = indexOne(opts, *gs, f.file);
                vector<ast::ParsedFile> toBeReResolved;
                toBeReResolved.emplace_back(move(reIndexed));
                auto reresolved = pipeline::incrementalResolve(*gs, move(toBeReResolved), opts);
                ENFORCE(reresolved.size() == 1);
                f = checkNoDefinitionsInsideProhibitedLines(*gs, move(reresolved[0]), 0, prohibitedLines);
            }
            ENFORCE(symbolsBefore == gs->symbolsUsed(),
                    "Stressing the incremental resolver should not add any new symbols");
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs->beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    gs->errorQueue->flushErrors();
    if (opts.print.ResolveTree.enabled || opts.print.ResolveTreeRaw.enabled) {
        for (auto &resolved : what) {
            if (opts.print.ResolveTree.enabled) {
                opts.print.ResolveTree.fmt("{}\n", resolved.tree->toString(*gs));
            }
            if (opts.print.ResolveTreeRaw.enabled) {
                opts.print.ResolveTreeRaw.fmt("{}\n", resolved.tree->showRaw(*gs));
            }
        }
    }
    if (opts.print.MissingConstants.enabled) {
        what = printMissingConstants(*gs, opts, move(what));
    }

    return ast::ParsedFilesOrCancelled(move(what));
}

ast::ParsedFilesOrCancelled typecheck(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                      const options::Options &opts, WorkerPool &workers, bool cancelable,
                                      optional<shared_ptr<core::lsp::PreemptionTaskManager>> preemptionManager) {
    vector<ast::ParsedFile> typecheck_result;
    const auto &epochManager = *gs->epochManager;
    // Record epoch at start of typechecking before any preemption occurs.
    const u4 epoch = epochManager.getStatus().epoch;

    {
        Timer timeit(gs->tracer(), "typecheck");
        if (preemptionManager) {
            // Before kicking off typechecking, check if we need to preempt.
            (*preemptionManager)->tryRunScheduledPreemptionTask(*gs);
        }

        shared_ptr<ConcurrentBoundedQueue<ast::ParsedFile>> fileq;
        shared_ptr<BlockingBoundedQueue<typecheck_thread_result>> resultq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            resultq = make_shared<BlockingBoundedQueue<typecheck_thread_result>>(what.size());
        }

        core::Context ctx(*gs, core::Symbols::root());

        // We want to start typeckecking big files first because it helps with better work distribution
        fast_sort(what, [&](const auto &lhs, const auto &rhs) -> bool {
            return lhs.file.data(*gs).source().size() > rhs.file.data(*gs).source().size();
        });
        for (auto &resolved : what) {
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob(
                "typecheck", [ctx, &opts, epoch, &epochManager, &preemptionManager, fileq, resultq, cancelable]() {
                    typecheck_thread_result threadResult;
                    ast::ParsedFile job;
                    int processedByThread = 0;

                    {
                        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                            if (result.gotItem()) {
                                unique_ptr<absl::ReaderMutexLock> lock;
                                if (preemptionManager) {
                                    // [IDE] While held, no preemption tasks can run. Auto-released after each turn of
                                    // the loop.
                                    lock = (*preemptionManager)->lockPreemption();
                                }
                                processedByThread++;
                                // [IDE] Only do the work if typechecking hasn't been canceled.
                                const bool isCanceled = cancelable && epochManager.wasTypecheckingCanceled();
                                // [IDE] Also, don't do work if the file has changed under us since we began
                                // typechecking!
                                // TODO(jvilk): epoch is unlikely to overflow, but it is theoretically possible.
                                const bool fileWasChanged = preemptionManager && job.file.data(ctx).epoch > epoch;
                                if (!isCanceled && !fileWasChanged) {
                                    core::FileRef file = job.file;
                                    try {
                                        threadResult.trees.emplace_back(typecheckOne(ctx, move(job), opts));
                                    } catch (SorbetException &) {
                                        Exception::failInFuzzer();
                                        ctx.state.tracer().error("Exception typing file: {} (backtrace is above)",
                                                                 file.data(ctx).path());
                                    }
                                }
                            }
                        }
                    }
                    if (processedByThread > 0) {
                        threadResult.counters = getAndClearThreadCounters();
                        resultq->push(move(threadResult), processedByThread);
                    }
                });

            typecheck_thread_result threadResult;
            {
                for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs->tracer());
                     !result.done();
                     result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs->tracer())) {
                    if (result.gotItem()) {
                        counterConsume(move(threadResult.counters));
                        typecheck_result.insert(typecheck_result.end(), make_move_iterator(threadResult.trees.begin()),
                                                make_move_iterator(threadResult.trees.end()));
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());
                    gs->errorQueue->flushErrors();
                    if (preemptionManager) {
                        (*preemptionManager)->tryRunScheduledPreemptionTask(*gs);
                    }
                }
                if (cancelable && epochManager.wasTypecheckingCanceled()) {
                    return ast::ParsedFilesOrCancelled();
                }
            }
        }
        for (auto &extension : gs->semanticExtensions) {
            extension->finishTypecheck(*gs);
        }

        if (opts.print.SymbolTable.enabled) {
            opts.print.SymbolTable.fmt("{}\n", gs->toString());
        }
        if (opts.print.SymbolTableRaw.enabled) {
            opts.print.SymbolTableRaw.fmt("{}\n", gs->showRaw());
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.print.SymbolTableJson.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
            if (opts.print.SymbolTableJson.outputPath.empty()) {
                core::Proto::toJSON(root, cout);
            } else {
                stringstream buf;
                core::Proto::toJSON(root, buf);
                opts.print.SymbolTableJson.print(buf.str());
            }
        }
        if (opts.print.SymbolTableFullJson.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
            if (opts.print.SymbolTableJson.outputPath.empty()) {
                core::Proto::toJSON(root, cout);
            } else {
                stringstream buf;
                core::Proto::toJSON(root, buf);
                opts.print.SymbolTableJson.print(buf.str());
            }
        }
#endif
        if (opts.print.SymbolTableFull.enabled) {
            opts.print.SymbolTableFull.fmt("{}\n", gs->toStringFull());
        }
        if (opts.print.SymbolTableFullRaw.enabled) {
            opts.print.SymbolTableFullRaw.fmt("{}\n", gs->showRawFull());
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.print.FileTableJson.enabled) {
            auto files = core::Proto::filesToProto(*gs);
            if (opts.print.FileTableJson.outputPath.empty()) {
                core::Proto::toJSON(files, cout);
            } else {
                stringstream buf;
                core::Proto::toJSON(files, buf);
                opts.print.FileTableJson.print(buf.str());
            }
        }
        if (opts.print.PluginGeneratedCode.enabled) {
            plugin::Plugins::dumpPluginGeneratedFiles(*gs, opts.print.PluginGeneratedCode);
        }
#endif
        return ast::ParsedFilesOrCancelled(move(typecheck_result));
    }
}

class AllNamesCollector {
public:
    core::UsageHash acc;
    unique_ptr<ast::Send> preTransformSend(core::Context ctx, unique_ptr<ast::Send> original) {
        acc.sends.emplace_back(ctx, original->fun.data(ctx));
        return original;
    }

    unique_ptr<ast::MethodDef> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> original) {
        acc.constants.emplace_back(ctx, original->name.data(ctx));
        return original;
    }

    void handleUnresolvedConstantLit(core::Context ctx, ast::UnresolvedConstantLit *expr) {
        while (expr) {
            acc.constants.emplace_back(ctx, expr->cnst.data(ctx));
            // Handle references to 'Foo' in 'Foo::Bar'.
            expr = ast::cast_tree<ast::UnresolvedConstantLit>(expr->scope.get());
        }
    }

    unique_ptr<ast::ClassDef> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> original) {
        acc.constants.emplace_back(ctx, original->symbol.data(ctx)->name.data(ctx));
        original->name->showRaw(ctx);

        handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(original->name.get()));

        // Grab names of superclasses. (N.B. `include` and `extend` are captured as ConstantLits.)
        for (auto &ancst : original->ancestors) {
            handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(ancst.get()));
        }

        return original;
    }

    unique_ptr<ast::UnresolvedConstantLit>
    postTransformUnresolvedConstantLit(core::Context ctx, unique_ptr<ast::UnresolvedConstantLit> original) {
        handleUnresolvedConstantLit(ctx, original.get());
        return original;
    }

    unique_ptr<ast::UnresolvedIdent> postTransformUnresolvedIdent(core::Context ctx,
                                                                  unique_ptr<ast::UnresolvedIdent> id) {
        if (id->kind != ast::UnresolvedIdent::Kind::Local) {
            acc.constants.emplace_back(ctx, id->name.data(ctx));
        }
        return id;
    }
};

core::UsageHash getAllNames(const core::GlobalState &gs, unique_ptr<ast::Expression> &tree) {
    AllNamesCollector collector;
    tree = ast::TreeMap::apply(core::Context(gs, core::Symbols::root()), collector, move(tree));
    core::NameHash::sortAndDedupe(collector.acc.sends);
    core::NameHash::sortAndDedupe(collector.acc.constants);
    return move(collector.acc);
};

namespace {
core::FileHash computeFileHash(shared_ptr<core::File> forWhat, spdlog::logger &logger) {
    Timer timeit(logger, "computeFileHash");
    const static options::Options emptyOpts{};
    unique_ptr<core::GlobalState> lgs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(logger, logger)));
    lgs->initEmpty();
    lgs->errorQueue->ignoreFlushes = true;
    lgs->silenceErrors = true;
    core::FileRef fref;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        fref = lgs->enterFile(forWhat);
        fref.data(*lgs).strictLevel = pipeline::decideStrictLevel(*lgs, fref, emptyOpts);
    }
    vector<ast::ParsedFile> single;

    single.emplace_back(pipeline::indexOne(emptyOpts, *lgs, fref));
    auto errs = lgs->errorQueue->drainAllErrors();
    auto allNames = getAllNames(*lgs, single[0].tree);
    auto workers = WorkerPool::create(0, lgs->tracer());
    pipeline::resolve(lgs, move(single), emptyOpts, *workers, true);

    return {move(*lgs->hash()), move(allNames)};
}
}; // namespace

void computeFileHashes(const vector<shared_ptr<core::File>> files, spdlog::logger &logger, WorkerPool &workers) {
    Timer timeit(logger, "computeFileHashes");
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    logger.debug("Computing state hashes for {} files", files.size());

    shared_ptr<BlockingBoundedQueue<vector<pair<int, unique_ptr<const core::FileHash>>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<pair<int, unique_ptr<const core::FileHash>>>>>(files.size());
    workers.multiplexJob("lspStateHash", [fileq, resultq, files, &logger]() {
        vector<pair<int, unique_ptr<const core::FileHash>>> threadResult;
        int processedByThread = 0;
        int job;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job] || files[job]->getFileHash() != nullptr) {
                        continue;
                    }

                    threadResult.emplace_back(job, make_unique<core::FileHash>(computeFileHash(files[job], logger)));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        vector<pair<int, unique_ptr<const core::FileHash>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger); !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), logger)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    files[a.first]->setFileHash(move(a.second));
                }
            }
        }
    }
}

bool cacheTreesAndFiles(const core::GlobalState &gs, vector<ast::ParsedFile> &parsedFiles,
                        const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return false;
    }

    Timer timeit(gs.tracer(), "pipeline::cacheTreesAndFiles");
    int i = -1;
    bool written = false;
    for (auto &pfile : parsedFiles) {
        i++;
        if (!pfile.file.exists()) {
            continue;
        }

        auto &file = pfile.file.data(gs);
        if (!file.cached && !file.hasParseErrors) {
            kvstore->write(fileKey(file), core::serialize::Serializer::storeFile(file, pfile));
            written = true;
        }
    }
    return written;
}

} // namespace sorbet::realmain::pipeline
