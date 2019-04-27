// has to go first, as it violates poisons
#include "core/proto/proto.h"

#include "ProgressIndicator.h"
#include "absl/strings/escaping.h" // BytesToHexString
#include "ast/desugar/Desugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "core/Unfreeze.h"
#include "core/errors/parser.h"
#include "core/serialize/serialize.h"
#include "dsl/dsl.h"
#include "infer/infer.h"
#include "namer/configatron/configatron.h"
#include "namer/namer.h"
#include "parser/parser.h"
#include "pipeline.h"
#include "plugin/Plugins.h"
#include "plugin/SubprocessTextPlugin.h"
#include "resolver/resolver.h"

using namespace std;

namespace sorbet::realmain::pipeline {

class CFGCollectorAndTyper {
    const options::Options &opts;

public:
    CFGCollectorAndTyper(const options::Options &opts) : opts(opts){};

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> m) {
        if (m->loc.file().data(ctx).strictLevel < core::StrictLevel::Typed || m->symbol.data(ctx)->isOverloaded()) {
            return m;
        }
        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m->symbol), *m);

        if (opts.stopAfterPhase == options::Phase::CFG) {
            return m;
        }
        cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
        if (print.CFG) {
            fmt::print("{}\n\n", cfg->toString(ctx));
        }
        return m;
    }
};

string fileKey(core::GlobalState &gs, core::FileRef file) {
    auto path = file.data(gs).path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = sorbet::crypto_hashing::hash64(file.data(gs).source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

unique_ptr<ast::Expression> fetchTreeFromCache(core::GlobalState &gs, core::FileRef file,
                                               const unique_ptr<KeyValueStore> &kvstore) {
    if (kvstore && file.id() < gs.filesUsed()) {
        string fileHashKey = fileKey(gs, file);
        auto maybeCached = kvstore->read(fileHashKey);
        if (maybeCached) {
            prodCounterInc("types.input.files.kvstore.hit");
            auto cachedTree = core::serialize::Serializer::loadExpression(gs, maybeCached, file.id());
            file.data(gs).cachedParseTree = true;
            ENFORCE(cachedTree->loc.file() == file);
            return cachedTree;
        } else {
            prodCounterInc("types.input.files.kvstore.miss");
        }
    }
    return nullptr;
}

unique_ptr<parser::Node> runParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print) {
    Timer timeit(gs.tracer(), "runParser", {{"file", file.data(gs).path()}});
    unique_ptr<parser::Node> nodes;
    {
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        nodes = parser::Parser::run(gs, file);
    }
    if (print.ParseTree) {
        fmt::print("{}\n", nodes->toStringWithTabs(gs, 0));
    }
    if (print.ParseTreeJSON) {
        fmt::print("{}\n", nodes->toJSON(gs, 0));
    }
    return nodes;
}

unique_ptr<ast::Expression> runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                                       const options::Printers &print) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", file.data(gs).path()}});
    unique_ptr<ast::Expression> ast;
    core::MutableContext ctx(gs, core::Symbols::root());
    {
        core::ErrorRegion errs(gs, file);
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(ctx, move(parseTree));
    }
    if (print.Desugared) {
        fmt::print("{}\n", ast->toStringWithTabs(gs, 0));
    }
    if (print.DesugaredRaw) {
        fmt::print("{}\n", ast->showRaw(gs));
    }
    return ast;
}

unique_ptr<ast::Expression> runDSL(core::GlobalState &gs, core::FileRef file, unique_ptr<ast::Expression> ast) {
    core::MutableContext ctx(gs, core::Symbols::root());
    Timer timeit(gs.tracer(), "runDSL", {{"file", file.data(gs).path()}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
    core::ErrorRegion errs(gs, file);
    return dsl::DSL::run(ctx, move(ast));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {make_unique<ast::EmptyTree>(), file};
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         unique_ptr<KeyValueStore> &kvstore) {
    auto &print = opts.print;
    ast::ParsedFile dslsInlined{nullptr, file};

    Timer timeit(lgs.tracer(), "indexOne");
    try {
        unique_ptr<ast::Expression> tree = fetchTreeFromCache(lgs, file, kvstore);

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
            if (!opts.skipDSLPasses) {
                tree = runDSL(lgs, file, move(tree));
            }
        }
        if (print.DSLTree) {
            fmt::print("{}\n", tree->toStringWithTabs(lgs, 0));
        }
        if (print.DSLTreeRaw) {
            fmt::print("{}\n", tree->showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::DSL) {
            return emptyParsedFile(file);
        }

        dslsInlined.tree = move(tree);
        return dslsInlined;
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
                                                                          unique_ptr<KeyValueStore> &kvstore) {
    auto &print = opts.print;
    ast::ParsedFile dslsInlined{nullptr, file};
    vector<shared_ptr<core::File>> resultPluginFiles;

    Timer timeit(gs.tracer(), "indexOneWithPlugins", {{"file", file.data(gs).path()}});
    try {
        unique_ptr<ast::Expression> tree = fetchTreeFromCache(gs, file, kvstore);

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
            {
                Timer timeit(gs.tracer(), "plugins_text");
                core::MutableContext ctx(gs, core::Symbols::root());
                core::ErrorRegion errs(gs, file);
                auto [pluginTree, pluginFiles] = plugin::SubprocessTextPlugin::run(ctx, move(tree));
                tree = move(pluginTree);
                resultPluginFiles = move(pluginFiles);
            }
            if (!opts.skipDSLPasses) {
                tree = runDSL(gs, file, move(tree));
            }
        }
        if (print.DSLTree) {
            fmt::print("{}\n", tree->toStringWithTabs(gs, 0));
        }
        if (print.DSLTreeRaw) {
            fmt::print("{}\n", tree->showRaw(gs));
        }
        if (opts.stopAfterPhase == options::Phase::DSL) {
            return emptyPluginFile(file);
        }

        dslsInlined.tree = move(tree);
        return {move(dslsInlined), resultPluginFiles};
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
        int i = 0;
        Timer timeit(gs.tracer(), "incremental_naming");
        for (auto &tree : what) {
            auto file = tree.file;
            try {
                unique_ptr<ast::Expression> ast;
                core::MutableContext ctx(gs, core::Symbols::root());
                gs.tracer().trace("Naming: {}", file.data(gs).path());
                core::ErrorRegion errs(gs, file);
                core::UnfreezeSymbolTable symbolTable(gs);
                core::UnfreezeNameTable nameTable(gs);
                tree = sorbet::namer::Namer::run(ctx, move(tree));
                i++;
            } catch (SorbetException &) {
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(gs).path());
                }
            }
        }

        core::MutableContext ctx(gs, core::Symbols::root());
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
    auto fnd = opts.strictnessOverrides.find(string(fileData.path()));
    if (fnd != opts.strictnessOverrides.end()) {
        if (fnd->second == fileData.originalSigil) {
            core::ErrorRegion errs(gs, file);
            if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Parser::ParserError)) {
                e.setHeader("Useless override of strictness level");
            }
        }
        level = fnd->second;
    } else {
        if (fileData.originalSigil == core::StrictLevel::None) {
            level = core::StrictLevel::Stripe;
        } else {
            level = fileData.originalSigil;
        }
    }

    core::StrictLevel minStrict = opts.forceMinStrict;
    core::StrictLevel maxStrict = opts.forceMaxStrict;
    if (level <= core::StrictLevel::Max && level > core::StrictLevel::Ignore) {
        level = max(min(level, maxStrict), minStrict);
    }

    if (gs.runningUnderAutogen) {
        // Autogen stops before infer but needs to see all definitions
        level = core::StrictLevel::Stripe;
    }

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
        case core::StrictLevel::Stripe:
            prodCounterInc("types.input.files.sigil.none");
            break;
        case core::StrictLevel::Typed:
            prodCounterInc("types.input.files.sigil.typed");
            break;
        case core::StrictLevel::Strict:
            prodCounterInc("types.input.files.sigil.strictLevel");
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
    }

    return level;
}

void readFileWithStrictnessOverrides(unique_ptr<core::GlobalState> &gs, core::FileRef file,
                                     const options::Options &opts) {
    if (file.dataAllowingUnsafe(*gs).sourceType != core::File::NotYetRead) {
        return;
    }
    auto fileName = file.dataAllowingUnsafe(*gs).path();
    Timer timeit(gs->tracer(), "readFileWithStrictnessOverrides", {{"file", fileName}});
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
        auto entered = gs->enterNewFileAt(
            make_shared<core::File>(string(fileName.begin(), fileName.end()), move(src), core::File::Normal), file);
        ENFORCE(entered == file);
    }
    if (enable_counters) {
        counterAdd("types.input.lines", file.data(*gs).lineCount());
    }

    auto &fileData = file.data(*gs);
    if (!fileFound) {
        if (auto e = gs->beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("File Not Found");
        }
    }

    if (!opts.storeState.empty()) {
        fileData.sourceType = core::File::PayloadGeneration;
    }

    fileData.strictLevel = decideStrictLevel(*gs, file, opts);
}

struct IndexResult {
    unique_ptr<core::GlobalState> gs;
    vector<ast::ParsedFile> trees;
    vector<shared_ptr<core::File>> pluginGeneratedFiles;
};

struct IndexThreadResultPack {
    CounterState counters;
    IndexResult res;
    FlowId flow;
};

IndexResult mergeIndexResults(const shared_ptr<core::GlobalState> cgs, const options::Options &opts,
                              shared_ptr<BlockingBoundedQueue<IndexThreadResultPack>> input,
                              unique_ptr<KeyValueStore> &kvstore) {
    ProgressIndicator progress(opts.showProgress, "Indexing", input->bound);
    Timer timeit(cgs->tracer(), "mergeIndexResults");
    IndexThreadResultPack threadResult;
    IndexResult ret;
    for (auto result = input->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS, cgs->tracer()); !result.done();
         result = input->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS, cgs->tracer())) {
        if (result.gotItem()) {
            counterConsume(move(threadResult.counters));
            if (ret.gs == nullptr) {
                ret.gs = move(threadResult.res.gs);
                ENFORCE(ret.trees.empty());
                ret.trees = move(threadResult.res.trees);
                ret.pluginGeneratedFiles = move(threadResult.res.pluginGeneratedFiles);
            } else {
                Timer timeit(cgs->tracer(), "substitute");
                timingAddFlowEnd(threadResult.flow);
                core::GlobalSubstitution substitution(*threadResult.res.gs, *ret.gs, cgs.get());
                core::MutableContext ctx(*ret.gs, core::Symbols::root());
                for (auto &tree : threadResult.res.trees) {
                    auto file = tree.file;
                    core::ErrorRegion errs(*ret.gs, file);
                    if (!file.data(*ret.gs).cachedParseTree) {
                        tree.tree = ast::Substitute::run(ctx, substitution, move(tree.tree));
                        if (kvstore) {
                            string fileHashKey = fileKey(*ret.gs, file);
                            kvstore->write(fileHashKey,
                                           core::serialize::Serializer::storeExpression(*ret.gs, tree.tree));
                        }
                    }
                    ret.trees.emplace_back(move(tree));
                }
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
                               const options::Options &opts, WorkerPool &workers, unique_ptr<KeyValueStore> &kvstore) {
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
                    readFileWithStrictnessOverrides(localGs, file, opts);
                    auto [parsedFile, pluginFiles] = indexOneWithPlugins(opts, *localGs, file, kvstore);
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
            threadResult.flow = timingAddFlowStart("indexSuppliedFilesWorker");
            resultq->push(move(threadResult), threadResult.res.trees.size());
        }
    });

    return mergeIndexResults(baseGs, opts, resultq, kvstore);
}

IndexResult indexPluginFiles(IndexResult firstPass, const options::Options &opts, WorkerPool &workers,
                             unique_ptr<KeyValueStore> &kvstore) {
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
                threadResult.res.trees.emplace_back(indexOne(opts, *localGs, file, kvstore));
            }
        }

        if (!threadResult.res.trees.empty()) {
            threadResult.counters = getAndClearThreadCounters();
            threadResult.res.gs = move(localGs);
            threadResult.flow = timingAddFlowStart("indexPluginFilesWorker");
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
                              const options::Options &opts, WorkerPool &workers, unique_ptr<KeyValueStore> &kvstore) {
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
            readFileWithStrictnessOverrides(gs, file, opts);
            auto [parsedFile, pluginFiles] = indexOneWithPlugins(opts, *gs, file, kvstore);
            ret.emplace_back(move(parsedFile));
            pluginFileCount += pluginFiles.size();
            for (auto &pluginFile : pluginFiles) {
                core::FileRef pluginFileRef;
                {
                    core::UnfreezeFileTable fileTableAccess(*gs);
                    pluginFileRef = gs->enterFile(pluginFile);
                    pluginFileRef.data(*gs).strictLevel = decideStrictLevel(*gs, pluginFileRef, opts);
                }
                ret.emplace_back(indexOne(opts, *gs, pluginFileRef, kvstore));
            }
        }
        ENFORCE(files.size() + pluginFileCount == ret.size());
    } else {
        auto firstPass = indexSuppliedFiles(move(gs), files, opts, workers, kvstore);

        Timer timeit(firstPass.gs->tracer(), "index-rem");
        auto pluginPass = indexPluginFiles(move(firstPass), opts, workers, kvstore);
        {
            Timer timeit(pluginPass.gs->tracer(), "moving gs?");
            gs = move(pluginPass.gs);
        }
        {
            Timer timeit(gs->tracer(), "moving trees?");
            ret = move(pluginPass.trees);
        }
    }

    {
        Timer timeit(gs->tracer(), "sortingFiles");
        fast_sort(ret, [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
    }
    return ret;
}

ast::ParsedFile typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts) {
    ast::ParsedFile result{make_unique<ast::EmptyTree>(), resolved.file};
    core::FileRef f = resolved.file;
    if (opts.stopAfterPhase == options::Phase::NAMER || opts.stopAfterPhase == options::Phase::RESOLVER) {
        return result;
    }
    if (f.data(ctx).isRBI()) {
        return result;
    }

    Timer timeit(ctx.state.tracer(), "typecheckOne", {{"file", f.data(ctx).path()}});
    try {
        if (opts.print.CFG) {
            fmt::print("digraph \"{}\" {{\n", FileOps::getFileName(f.data(ctx).path()));
        }
        CFGCollectorAndTyper collector(opts);
        {
            core::ErrorRegion errs(ctx, f);
            result.tree = ast::TreeMap::apply(ctx, collector, move(resolved.tree));
        }
        if (opts.print.CFG) {
            fmt::print("}}\n\n");
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
        core::UnfreezeNameTable nameTableAccess(gs);     // creates names from config
        core::UnfreezeSymbolTable symbolTableAccess(gs); // creates methods for them
        namer::configatron::fillInFromFileSystem(gs, opts.configatronDirs, opts.configatronFiles);
    }

    {
        ProgressIndicator namingProgress(opts.showProgress, "Naming", what.size());

        int i = 0;
        for (auto &tree : what) {
            auto file = tree.file;
            try {
                ast::ParsedFile ast;
                {
                    core::MutableContext ctx(gs, core::Symbols::root());
                    Timer timeit(gs.tracer(), "naming", {{"file", file.data(gs).path()}});
                    core::ErrorRegion errs(gs, file);
                    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
                    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
                    tree = namer::Namer::run(ctx, move(tree));
                }
                gs.errorQueue->flushErrors();
                namingProgress.reportProgress(i);
                i++;
            } catch (SorbetException &) {
                Exception::failInFuzzer();
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(gs).path());
                }
            }
        }
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

vector<ast::ParsedFile> printMissingConstants(core::GlobalState &gs, vector<ast::ParsedFile> what) {
    Timer timeit(gs.tracer(), "printMissingConstants");
    core::MutableContext ctx(gs, core::Symbols::root());
    GatherUnresolvedConstantsWalk walk;
    for (auto &resolved : what) {
        resolved.tree = ast::TreeMap::apply(ctx, walk, move(resolved.tree));
    }
    fmt::print("{}\n", fmt::join(walk.unresolvedConstants, "\n"));
    return what;
}

vector<ast::ParsedFile> resolve(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                bool skipConfigatron) {
    try {
        what = name(gs, move(what), opts, skipConfigatron);

        for (auto &named : what) {
            if (opts.print.NameTree) {
                fmt::print("{}\n", named.tree->toStringWithTabs(gs, 0));
            }
            if (opts.print.NameTreeRaw) {
                fmt::print("{}\n", named.tree->showRaw(gs));
            }
        }

        if (opts.stopAfterPhase == options::Phase::NAMER) {
            return what;
        }

        core::MutableContext ctx(gs, core::Symbols::root());
        ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
        {
            Timer timeit(gs.tracer(), "resolving");
            vector<core::ErrorRegion> errs;
            for (auto &tree : what) {
                auto file = tree.file;
                errs.emplace_back(gs, file);
            }
            core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
            core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
            what = resolver::Resolver::run(ctx, move(what));
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }
    gs.errorQueue->flushErrors();
    if (opts.print.ResolveTree || opts.print.ResolveTreeRaw) {
        for (auto &resolved : what) {
            if (opts.print.ResolveTree) {
                fmt::print("{}\n", resolved.tree->toString(gs));
            }
            if (opts.print.ResolveTreeRaw) {
                fmt::print("{}\n", resolved.tree->showRaw(gs));
            }
        }
    }
    if (opts.print.MissingConstants) {
        what = printMissingConstants(gs, move(what));
    }
    return what;
}

vector<ast::ParsedFile> typecheck(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                  const options::Options &opts, WorkerPool &workers) {
    vector<ast::ParsedFile> typecheck_result;

    {
        Timer timeit(gs->tracer(), "typecheck");

        shared_ptr<ConcurrentBoundedQueue<ast::ParsedFile>> fileq;
        shared_ptr<BlockingBoundedQueue<typecheck_thread_result>> resultq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            resultq = make_shared<BlockingBoundedQueue<typecheck_thread_result>>(what.size());
        }

        core::Context ctx(*gs, core::Symbols::root());
        for (auto &resolved : what) {
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob("typecheck", [ctx, &opts, fileq, resultq]() {
                typecheck_thread_result threadResult;
                ast::ParsedFile job;
                int processedByThread = 0;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            processedByThread++;
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
                if (processedByThread > 0) {
                    threadResult.counters = getAndClearThreadCounters();
                    resultq->push(move(threadResult), processedByThread);
                }
            });

            typecheck_thread_result threadResult;
            {
                for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS, gs->tracer());
                     !result.done();
                     result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS, gs->tracer())) {
                    if (result.gotItem()) {
                        counterConsume(move(threadResult.counters));
                        typecheck_result.insert(typecheck_result.end(), make_move_iterator(threadResult.trees.begin()),
                                                make_move_iterator(threadResult.trees.end()));
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());
                    gs->errorQueue->flushErrors();
                }
            }
        }

        if (opts.print.SymbolTable) {
            fmt::print("{}\n", gs->toString());
        }
        if (opts.print.SymbolTableRaw) {
            fmt::print("{}\n", gs->toStringWithOptions(false, true));
        }
        if (opts.print.SymbolTableJson) {
            auto root = core::Proto::toProto(*gs, core::Symbols::root());
            core::Proto::toJSON(root, cout);
        }
        if (opts.print.SymbolTableFull) {
            fmt::print("{}\n", gs->toStringWithOptions(true, false));
        }
        if (opts.print.SymbolTableFullRaw) {
            fmt::print("{}\n", gs->toStringWithOptions(true, true));
        }
        if (opts.print.FileTableJson) {
            auto files = core::Proto::filesToProto(*gs);
            core::Proto::toJSON(files, cout);
        }
        if (opts.print.PluginGeneratedCode) {
            plugin::Plugins::dumpPluginGeneratedFiles(*gs);
        }

        return typecheck_result;
    }
}

unsigned int computeFileHash(shared_ptr<core::File> forWhat, spdlog::logger &logger) {
    Timer timeit(logger, "computeFileHash");
    const static options::Options emptyOpts{};
    shared_ptr<core::GlobalState> lgs = make_shared<core::GlobalState>((make_shared<core::ErrorQueue>(logger, logger)));
    lgs->initEmpty();
    lgs->errorQueue->ignoreFlushes = true;
    lgs->silenceErrors = true;
    core::FileRef fref;
    {
        core::UnfreezeFileTable fileTableAccess(*lgs);
        fref = lgs->enterFile(forWhat);
    }
    vector<ast::ParsedFile> single;
    unique_ptr<KeyValueStore> kvstore;

    single.emplace_back(pipeline::indexOne(emptyOpts, *lgs, fref, kvstore));
    auto errs = lgs->errorQueue->drainAllErrors();
    for (auto &e : errs) {
        if (e->what == core::errors::Parser::ParserError) {
            return core::GlobalState::HASH_STATE_INVALID;
        }
    }
    pipeline::resolve(*lgs, move(single), emptyOpts, true);

    return lgs->hash();
}

} // namespace sorbet::realmain::pipeline
