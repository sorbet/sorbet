#ifdef SORBET_REALMAIN_MIN
// minimal build to speedup compilation. Remove extra features
#else
// has to go first, as it violates poisons
#include "core/proto/proto.h"
// ^^ has to go first
#include "common/json2msgpack/json2msgpack.h"
#include "namer/configatron/configatron.h"
#include "packager/packager.h"
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
#include "core/NullFlusher.h"
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

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &m = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (ctx.file.data(ctx).strictLevel < core::StrictLevel::True || m.symbol.data(ctx)->isOverloaded()) {
            return tree;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m.symbol), m);

        if (opts.stopAfterPhase == options::Phase::CFG) {
            return tree;
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
        return tree;
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
            return make_unique<core::serialize::CachedFile>(move(cachedTree));
        } else {
            prodCounterInc("types.input.files.kvstore.miss");
        }
    }
    return nullptr;
}

ast::TreePtr fetchTreeFromCache(core::GlobalState &gs, core::FileRef fref, const core::File &file,
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

ast::TreePtr runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                        const options::Printers &print) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", (string)file.data(gs).path()}});
    ast::TreePtr ast;
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    {
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(ctx, move(parseTree));
    }
    if (print.DesugarTree.enabled) {
        print.DesugarTree.fmt("{}\n", ast->toStringWithTabs(gs, 0));
    }
    if (print.DesugarTreeRaw.enabled) {
        print.DesugarTreeRaw.fmt("{}\n", ast.showRaw(gs));
    }
    return ast;
}

ast::TreePtr runRewriter(core::GlobalState &gs, core::FileRef file, ast::TreePtr ast) {
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    Timer timeit(gs.tracer(), "runRewriter", {{"file", (string)file.data(gs).path()}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
    return rewriter::Rewriter::run(ctx, move(ast));
}

ast::ParsedFile runLocalVars(core::GlobalState &gs, ast::ParsedFile tree) {
    Timer timeit(gs.tracer(), "runLocalVars", {{"file", (string)tree.file.data(gs).path()}});
    core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
    return sorbet::local_vars::LocalVars::run(ctx, move(tree));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {ast::MK::EmptyTree(), file};
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file, ast::TreePtr tree) {
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
            print.RewriterTreeRaw.fmt("{}\n", tree.showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyParsedFile(file);
        }

        rewriten.tree = move(tree);
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

pair<ast::ParsedFile, vector<shared_ptr<core::File>>>
indexOneWithPlugins(const options::Options &opts, core::GlobalState &gs, core::FileRef file, ast::TreePtr tree) {
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
                core::MutableContext ctx(gs, core::Symbols::root(), file);

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
                print.RewriterTreeRaw.fmt("{}\n", tree.showRaw(gs));
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
            print.IndexTreeRaw.fmt("{}\n", tree.showRaw(gs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyPluginFile(file);
        }

        rewriten.tree = move(tree);
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
#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            Timer timeit(gs.tracer(), "incremental_packager");
            what = packager::Packager::runIncremental(gs, move(what));
        }
#endif
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);
            auto emptyWorkers = WorkerPool::create(0, gs.tracer());

            auto result = sorbet::namer::Namer::run(gs, move(what), *emptyWorkers);
            // Cancellation cannot occur during incremental namer.
            ENFORCE(result.hasResult());
            what = move(result.result());
        }

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            auto result = sorbet::resolver::Resolver::runTreePasses(gs, move(what));
            // incrementalResolve is not cancelable.
            ENFORCE(result.hasResult());
            what = move(result.result());
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
ast::TreePtr readFileWithStrictnessOverrides(unique_ptr<core::GlobalState> &gs, core::FileRef file,
                                             const options::Options &opts,
                                             const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    ast::TreePtr ast;
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
        if (auto maybeCached = fetchFileFromCache(*gs, file, *fileObj, kvstore)) {
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
                {
                    Timer timeit(cgs->tracer(), "substituteTrees");
                    for (auto &tree : threadResult.res.trees) {
                        auto file = tree.file;
                        if (!file.data(*ret.gs).cached) {
                            core::MutableContext ctx(*ret.gs, core::Symbols::root(), file);
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
        for (auto &tree : firstPass.trees) {
            auto file = tree.file;
            core::MutableContext ctx(*suppliedFilesAndPluginFiles.gs, core::Symbols::root(), file);
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
        opts.print.FlattenTreeRaw.fmt("{}\n", resolved.tree.showRaw(ctx));
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

vector<ast::ParsedFile> package(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    if (opts.stripePackages) {
        Timer timeit(gs.tracer(), "package");
        what = packager::Packager::run(gs, workers, move(what));
        if (opts.print.Packager.enabled) {
            for (auto &f : what) {
                opts.print.Packager.fmt("{}\n", f.tree->toStringWithTabs(gs, 0));
            }
        }
    }
#endif
    return what;
}

ast::ParsedFilesOrCancelled name(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                 WorkerPool &workers, bool skipConfigatron) {
    Timer timeit(gs.tracer(), "name");
    if (!skipConfigatron) {
#ifndef SORBET_REALMAIN_MIN
        core::UnfreezeNameTable nameTableAccess(gs);     // creates names from config
        core::UnfreezeSymbolTable symbolTableAccess(gs); // creates methods for them
        namer::configatron::fillInFromFileSystem(gs, opts.configatronDirs, opts.configatronFiles);
#endif
    }

    {
        core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
        core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
        auto result = namer::Namer::run(gs, move(what), workers);

        return result;
    }
}
class GatherUnresolvedConstantsWalk {
public:
    vector<string> unresolvedConstants;
    ast::TreePtr postTransformConstantLit(core::MutableContext ctx, ast::TreePtr tree) {
        auto unresolvedPath = ast::cast_tree_nonnull<ast::ConstantLit>(tree).fullUnresolvedPath(ctx);
        if (unresolvedPath.has_value()) {
            unresolvedConstants.emplace_back(fmt::format(
                "{}::{}",
                unresolvedPath->first != core::Symbols::root() ? unresolvedPath->first.data(ctx)->show(ctx) : "",
                fmt::map_join(unresolvedPath->second,
                              "::", [&](const auto &el) -> string { return el.data(ctx)->show(ctx); })));
        }
        return tree;
    }
};

vector<ast::ParsedFile> printMissingConstants(core::GlobalState &gs, const options::Options &opts,
                                              vector<ast::ParsedFile> what) {
    Timer timeit(gs.tracer(), "printMissingConstants");
    GatherUnresolvedConstantsWalk walk;
    for (auto &resolved : what) {
        core::MutableContext ctx(gs, core::Symbols::root(), resolved.file);
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

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        checkSym(ctx, ast::cast_tree_nonnull<ast::ClassDef>(tree).symbol);
        return tree;
    }
    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        checkSym(ctx, ast::cast_tree_nonnull<ast::MethodDef>(tree).symbol);
        return tree;
    }
};

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd) {
    DefinitionLinesBlacklistEnforcer enforcer(what.file, prohibitedLinesStart, prohibitedLinesEnd);
    what.tree = ast::TreeMap::apply(core::Context(gs, core::Symbols::root(), what.file), enforcer, move(what.tree));
    return what;
}

ast::ParsedFilesOrCancelled resolve(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers, bool skipConfigatron) {
    try {
        // packager intentionally runs outside of rewriter so that its output does not get cached.
        what = package(*gs, move(what), opts, workers);

        auto result = name(*gs, move(what), opts, workers, skipConfigatron);
        if (!result.hasResult()) {
            return result;
        }
        what = move(result.result());

        for (auto &named : what) {
            if (opts.print.NameTree.enabled) {
                opts.print.NameTree.fmt("{}\n", named.tree->toStringWithTabs(*gs, 0));
            }
            if (opts.print.NameTreeRaw.enabled) {
                opts.print.NameTreeRaw.fmt("{}\n", named.tree.showRaw(*gs));
            }
        }

        if (opts.stopAfterPhase == options::Phase::NAMER) {
            return ast::ParsedFilesOrCancelled(move(what));
        }

        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(gs->tracer(), "resolving");
            core::UnfreezeNameTable nameTableAccess(*gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(*gs); // enters stubs
            auto maybeResult = resolver::Resolver::run(*gs, move(what), workers);
            if (!maybeResult.hasResult()) {
                return maybeResult;
            }
            what = move(maybeResult.result());
        }
        if (opts.stressIncrementalResolver) {
            auto symbolsBefore = gs->symbolsUsedTotal();
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
            ENFORCE(symbolsBefore == gs->symbolsUsedTotal(),
                    "Stressing the incremental resolver should not add any new symbols");
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs->beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    if (opts.print.ResolveTree.enabled || opts.print.ResolveTreeRaw.enabled) {
        for (auto &resolved : what) {
            if (opts.print.ResolveTree.enabled) {
                opts.print.ResolveTree.fmt("{}\n", resolved.tree->toString(*gs));
            }
            if (opts.print.ResolveTreeRaw.enabled) {
                opts.print.ResolveTreeRaw.fmt("{}\n", resolved.tree.showRaw(*gs));
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
                                      optional<shared_ptr<core::lsp::PreemptionTaskManager>> preemptionManager,
                                      bool presorted) {
    // Unless the error queue had a critical error, only typecheck should flush errors to the client, otherwise we will
    // drop errors in LSP mode.
    ENFORCE(gs->hadCriticalError() || gs->errorQueue->filesFlushedCount == 0);

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
        shared_ptr<BlockingBoundedQueue<vector<ast::ParsedFile>>> treesq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            treesq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(what.size());
        }

        const core::GlobalState &igs = *gs;
        if (!presorted) {
            // If files are not already sorted, we want to start typeckecking big files first because it helps with
            // better work distribution
            fast_sort(what, [&](const auto &lhs, const auto &rhs) -> bool {
                return lhs.file.data(*gs).source().size() > rhs.file.data(*gs).source().size();
            });
        }

        for (auto &resolved : what) {
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob(
                "typecheck", [&igs, &opts, epoch, &epochManager, &preemptionManager, fileq, treesq, cancelable]() {
                    vector<ast::ParsedFile> trees;
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
                                const bool fileWasChanged = preemptionManager && job.file.data(igs).epoch > epoch;
                                if (!isCanceled && !fileWasChanged) {
                                    core::FileRef file = job.file;
                                    try {
                                        core::Context ctx(igs, core::Symbols::root(), file);
                                        auto parsedFile = typecheckOne(ctx, move(job), opts);
                                        trees.emplace_back(move(parsedFile));
                                    } catch (SorbetException &) {
                                        Exception::failInFuzzer();
                                        igs.tracer().error("Exception typing file: {} (backtrace is above)",
                                                           file.data(igs).path());
                                    }
                                    // Stream out errors
                                    treesq->push(move(trees), processedByThread);
                                    processedByThread = 0;
                                }
                            }
                        }
                    }
                    if (processedByThread > 0) {
                        treesq->push(move(trees), processedByThread);
                    }
                });

            vector<ast::ParsedFile> trees;
            {
                for (auto result = treesq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), gs->tracer());
                     !result.done();
                     result = treesq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), gs->tracer())) {
                    if (result.gotItem()) {
                        for (auto &tree : trees) {
                            gs->errorQueue->flushErrorsForFile(*gs, tree.file);
                        }
                        typecheck_result.insert(typecheck_result.end(), make_move_iterator(trees.begin()),
                                                make_move_iterator(trees.end()));
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());

                    if (preemptionManager) {
                        (*preemptionManager)->tryRunScheduledPreemptionTask(*gs);
                    }
                }
                if (cancelable && epochManager.wasTypecheckingCanceled()) {
                    return ast::ParsedFilesOrCancelled();
                }
            }

            if (workers.size() > 0) {
                auto counterq = make_shared<BlockingBoundedQueue<CounterState>>(workers.size());
                workers.multiplexJob("collectCounters",
                                     [counterq]() { counterq->push(getAndClearThreadCounters(), 1); });
                {
                    sorbet::CounterState counters;
                    for (auto result = counterq->try_pop(counters); !result.done();
                         result = counterq->try_pop(counters)) {
                        if (result.gotItem()) {
                            counterConsume(move(counters));
                        }
                    }
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
        if (opts.print.SymbolTableProto.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
            if (opts.print.SymbolTableProto.outputPath.empty()) {
                root.SerializeToOstream(&cout);
            } else {
                string buf;
                root.SerializeToString(&buf);
                opts.print.SymbolTableProto.print(buf);
            }
        }
        if (opts.print.SymbolTableMessagePack.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), false);
            stringstream buf;
            core::Proto::toJSON(root, buf);
            auto str = buf.str();
            rapidjson::Document document;
            document.Parse(str);
            mpack_writer_t writer;
            if (opts.print.SymbolTableMessagePack.outputPath.empty()) {
                mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
            } else {
                mpack_writer_init_filename(&writer, opts.print.SymbolTableMessagePack.outputPath.c_str());
            }
            json2msgpack::json2msgpack(document, &writer);
            if (mpack_writer_destroy(&writer)) {
                Exception::raise("failed to write msgpack");
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
        if (opts.print.SymbolTableFullProto.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
            if (opts.print.SymbolTableFullProto.outputPath.empty()) {
                root.SerializeToOstream(&cout);
            } else {
                string buf;
                root.SerializeToString(&buf);
                opts.print.SymbolTableFullProto.print(buf);
            }
        }
        if (opts.print.SymbolTableFullMessagePack.enabled) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root(), true);
            stringstream buf;
            core::Proto::toJSON(root, buf);
            auto str = buf.str();
            rapidjson::Document document;
            document.Parse(str);
            mpack_writer_t writer;
            if (opts.print.SymbolTableFullMessagePack.outputPath.empty()) {
                mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
            } else {
                mpack_writer_init_filename(&writer, opts.print.SymbolTableFullMessagePack.outputPath.c_str());
            }
            json2msgpack::json2msgpack(document, &writer);
            if (mpack_writer_destroy(&writer)) {
                Exception::raise("failed to write msgpack");
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
        if (opts.print.FileTableProto.enabled) {
            auto files = core::Proto::filesToProto(*gs);
            if (opts.print.FileTableProto.outputPath.empty()) {
                files.SerializeToOstream(&cout);
            } else {
                string buf;
                files.SerializeToString(&buf);
                opts.print.FileTableProto.print(buf);
            }
        }
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
        if (opts.print.FileTableMessagePack.enabled) {
            auto files = core::Proto::filesToProto(*gs);
            stringstream buf;
            core::Proto::toJSON(files, buf);
            auto str = buf.str();
            rapidjson::Document document;
            document.Parse(str);
            mpack_writer_t writer;
            if (opts.print.FileTableMessagePack.outputPath.empty()) {
                mpack_writer_init_stdfile(&writer, stdout, /* close when done */ false);
            } else {
                mpack_writer_init_filename(&writer, opts.print.FileTableMessagePack.outputPath.c_str());
            }
            json2msgpack::json2msgpack(document, &writer);
            if (mpack_writer_destroy(&writer)) {
                Exception::raise("failed to write msgpack");
            }
        }
        if (opts.print.PluginGeneratedCode.enabled) {
            plugin::Plugins::dumpPluginGeneratedFiles(*gs, opts.print.PluginGeneratedCode);
        }
#endif
        // Error queue is re-used across runs, so reset the flush count to ignore files flushed during typecheck.
        gs->errorQueue->filesFlushedCount = 0;

        return ast::ParsedFilesOrCancelled(move(typecheck_result));
    }
}

class AllNamesCollector {
public:
    core::UsageHash acc;
    ast::TreePtr preTransformSend(core::Context ctx, ast::TreePtr tree) {
        acc.sends.emplace_back(ctx, ast::cast_tree_nonnull<ast::Send>(tree).fun.data(ctx));
        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        acc.constants.emplace_back(ctx, original.name.data(ctx));
        return tree;
    }

    void handleUnresolvedConstantLit(core::Context ctx, ast::UnresolvedConstantLit *expr) {
        while (expr) {
            acc.constants.emplace_back(ctx, expr->cnst.data(ctx));
            // Handle references to 'Foo' in 'Foo::Bar'.
            expr = ast::cast_tree<ast::UnresolvedConstantLit>(expr->scope);
        }
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        acc.constants.emplace_back(ctx, original.symbol.data(ctx)->name.data(ctx));

        handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(original.name));

        // Grab names of superclasses. (N.B. `include` and `extend` are captured as ConstantLits.)
        for (auto &ancst : original.ancestors) {
            handleUnresolvedConstantLit(ctx, ast::cast_tree<ast::UnresolvedConstantLit>(ancst));
        }

        return tree;
    }

    ast::TreePtr postTransformUnresolvedConstantLit(core::Context ctx, ast::TreePtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(tree);
        handleUnresolvedConstantLit(ctx, &original);
        return tree;
    }

    ast::TreePtr postTransformUnresolvedIdent(core::Context ctx, ast::TreePtr tree) {
        auto &id = ast::cast_tree_nonnull<ast::UnresolvedIdent>(tree);
        if (id.kind != ast::UnresolvedIdent::Kind::Local) {
            acc.constants.emplace_back(ctx, id.name.data(ctx));
        }
        return tree;
    }
};

core::UsageHash getAllNames(core::Context ctx, ast::TreePtr &tree) {
    AllNamesCollector collector;
    tree = ast::TreeMap::apply(ctx, collector, move(tree));
    core::NameHash::sortAndDedupe(collector.acc.sends);
    core::NameHash::sortAndDedupe(collector.acc.constants);
    return move(collector.acc);
};

namespace {
core::FileHash computeFileHash(shared_ptr<core::File> forWhat, spdlog::logger &logger) {
    Timer timeit(logger, "computeFileHash");
    const static options::Options emptyOpts{};
    unique_ptr<core::GlobalState> lgs = make_unique<core::GlobalState>(
        (make_shared<core::ErrorQueue>(logger, logger, make_shared<core::NullFlusher>())));
    lgs->initEmpty();
    lgs->silenceErrors = true;
    core::FileRef fref;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        fref = lgs->enterFile(forWhat);
        fref.data(*lgs).strictLevel = pipeline::decideStrictLevel(*lgs, fref, emptyOpts);
    }
    vector<ast::ParsedFile> single;

    single.emplace_back(pipeline::indexOne(emptyOpts, *lgs, fref));
    core::Context ctx(*lgs, core::Symbols::root(), single[0].file);
    auto allNames = getAllNames(ctx, single[0].tree);
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

bool cacheTreesAndFiles(const core::GlobalState &gs, WorkerPool &workers, vector<ast::ParsedFile> &parsedFiles,
                        const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return false;
    }

    Timer timeit(gs.tracer(), "pipeline::cacheTreesAndFiles");

    // Compress files in parallel.
    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile *>>(parsedFiles.size());
    for (auto &parsedFile : parsedFiles) {
        auto ptr = &parsedFile;
        fileq->push(move(ptr), 1);
    }

    auto resultq = make_shared<BlockingBoundedQueue<vector<pair<string, vector<u1>>>>>(parsedFiles.size());
    workers.multiplexJob("compressTreesAndFiles", [fileq, resultq, &gs]() {
        vector<pair<string, vector<u1>>> threadResult;
        int processedByThread = 0;
        ast::ParsedFile *job = nullptr;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!job->file.exists()) {
                        continue;
                    }

                    auto &file = job->file.data(gs);
                    if (!file.cached && !file.hasParseErrors) {
                        threadResult.emplace_back(fileKey(file), core::serialize::Serializer::storeFile(file, *job));
                    }
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    bool written = false;
    {
        vector<pair<string, vector<u1>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    kvstore->write(move(a.first), move(a.second));
                    written = true;
                }
            }
        }
    }
    return written;
}

} // namespace sorbet::realmain::pipeline
