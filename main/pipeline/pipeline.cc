#ifdef SORBET_REALMAIN_MIN
// minimal build to speedup compilation. Remove extra features
#else
// has to go first, as it violates poisons
#include "core/proto/proto.h"
// ^^ has to go first
#include "common/json2msgpack/json2msgpack.h"
#include "rapidjson/writer.h"
#include <sstream>
#endif
#include "ProgressIndicator.h"
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/desugar/Desugar.h"
#include "ast/desugar/PrismDesugar.h"
#include "ast/substitute/substitute.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "class_flatten/class_flatten.h"
#include "common/FileOps.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/timers/Timer.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
#include "core/Unfreeze.h"
#include "core/errors/infer.h"
#include "core/errors/namer.h"
#include "core/errors/parser.h"
#include "core/errors/resolver.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "core/serialize/serialize.h"
#include "definition_validator/validator.h"
#include "infer/infer.h"
#include "local_vars/local_vars.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include "namer/namer.h"
#include "packager/packager.h"
#include "parser/parser.h"
#include "parser/prism/Parser.h"
#include "parser/prism/Translator.h"
#include "payload/binary/binary.h"
#include "pipeline.h"
#include "rbs/AssertionsRewriter.h"
#include "rbs/CommentsAssociator.h"
#include "rbs/SigsRewriter.h"
#include "rbs/prism/AssertionsRewriterPrism.h"
#include "rbs/prism/CommentsAssociatorPrism.h"
#include "rbs/prism/SigsRewriterPrism.h"
#include "resolver/resolver.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::realmain::pipeline {

void setGlobalStateOptions(core::GlobalState &gs, const options::Options &opts) {
    gs.pathPrefix = opts.pathPrefix;
    gs.errorUrlBase = opts.errorUrlBase;

    gs.cacheSensitiveOptions.rbsEnabled = opts.cacheSensitiveOptions.rbsEnabled;
    gs.cacheSensitiveOptions.requiresAncestorEnabled = opts.cacheSensitiveOptions.requiresAncestorEnabled;
    gs.cacheSensitiveOptions.rspecRewriterEnabled = opts.cacheSensitiveOptions.rspecRewriterEnabled;
    gs.cacheSensitiveOptions.typedSuper = opts.cacheSensitiveOptions.typedSuper;

    if (opts.silenceErrors) {
        gs.silenceErrors = true;
    }
    gs.autocorrect = opts.autocorrect;
    gs.didYouMean = opts.didYouMean;
    if (opts.censorForSnapshotTests) {
        gs.censorForSnapshotTests = true;
    }
    gs.sleepInSlowPathSeconds = opts.sleepInSlowPathSeconds;
    for (auto code : opts.isolateErrorCode) {
        gs.onlyShowErrorClass(code);
    }
    for (auto code : opts.suppressErrorCode) {
        gs.suppressErrorClass(code);
    }
    if (opts.noErrorSections) {
        gs.includeErrorSections = false;
    }
    gs.parseWithPrism = opts.cacheSensitiveOptions.usePrismParser;
    gs.ruby3KeywordArgs = opts.ruby3KeywordArgs;
    gs.suppressPayloadSuperclassRedefinitionFor = opts.suppressPayloadSuperclassRedefinitionFor;
    if (!opts.uniquelyDefinedBehavior) {
        // Definitions in multiple locations interact poorly with autoloader this error is enforced in Stripe code.
        if (opts.isolateErrorCode.empty()) {
            gs.suppressErrorClass(core::errors::Namer::MultipleBehaviorDefs.code);
        }
    }

    if (!opts.outOfOrderReferenceChecksEnabled) {
        if (opts.isolateErrorCode.empty()) {
            gs.suppressErrorClass(core::errors::Resolver::OutOfOrderConstantAccess.code);
        }
    }

    gs.trackUntyped = opts.trackUntyped;
    gs.printingFileTable = opts.print.FileTableJson.enabled || opts.print.FileTableFullJson.enabled ||
                           opts.print.FileTableProto.enabled || opts.print.FileTableFullProto.enabled ||
                           opts.print.FileTableMessagePack.enabled || opts.print.FileTableFullMessagePack.enabled;

    if (opts.suggestTyped) {
        gs.ignoreErrorClassForSuggestTyped(core::errors::Infer::SuggestTyped.code);
        gs.ignoreErrorClassForSuggestTyped(core::errors::Resolver::SigInFileWithoutSigil.code);
        if (!opts.uniquelyDefinedBehavior) {
            gs.ignoreErrorClassForSuggestTyped(core::errors::Namer::MultipleBehaviorDefs.code);
        }
    }
    gs.suggestUnsafe = opts.suggestUnsafe;

#ifndef SORBET_REALMAIN_MIN
    if (opts.cacheSensitiveOptions.sorbetPackages) {
        core::UnfreezeNameTable unfreezeToEnterPackagerOptionsGS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions(
            opts.extraPackageFilesDirectoryUnderscorePrefixes, opts.extraPackageFilesDirectorySlashDeprecatedPrefixes,
            opts.extraPackageFilesDirectorySlashPrefixes, opts.packageSkipRBIExportEnforcementDirs,
            opts.allowRelaxedPackagerChecksFor, opts.packagerLayers, opts.sorbetPackagesHint, opts.genPackages);
    }
#endif
}

unique_ptr<core::GlobalState> copyForSlowPath(const core::GlobalState &from, const options::Options &opts) {
    if (opts.cacheSensitiveOptions.noStdlib) {
        auto result = make_unique<core::GlobalState>(from.errorQueue, from.epochManager);
        result->initEmpty();
        return result;
    }

    auto result = from.copyForSlowPath(
        opts.extraPackageFilesDirectoryUnderscorePrefixes, opts.extraPackageFilesDirectorySlashDeprecatedPrefixes,
        opts.extraPackageFilesDirectorySlashPrefixes, opts.packageSkipRBIExportEnforcementDirs,
        opts.allowRelaxedPackagerChecksFor, opts.packagerLayers, opts.sorbetPackagesHint, opts.genPackages);

    core::serialize::Serializer::loadSymbolTable(*result, PAYLOAD_SYMBOL_TABLE);

    return result;
}

vector<core::FileRef> reserveFiles(core::GlobalState &gs, const vector<string> &files) {
    Timer timeit(gs.tracer(), "reserveFiles");
    vector<core::FileRef> ret;
    ret.reserve(files.size());
    core::UnfreezeFileTable unfreezeFiles(gs);
    for (auto &f : files) {
        auto fileRef = gs.findFileByPath(f);
        if (!fileRef.exists()) {
            fileRef = gs.reserveFileRef(f);
        }
        ret.emplace_back(move(fileRef));
    }
    return ret;
}

// ----- indexer --------------------------------------------------------------

core::StrictLevel decideStrictLevel(const core::GlobalState &gs, const core::FileRef file,
                                    const options::Options &opts) {
    auto &fileData = file.data(gs);

    core::StrictLevel level;

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

    if (!opts.strictnessOverrides.empty()) {
        string filePath = string(fileData.path());
        // make sure all relative file paths start with ./
        if (!absl::StartsWith(filePath, "/") && !absl::StartsWith(filePath, "./")) {
            filePath.insert(0, "./");
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
    }

    if (gs.cacheSensitiveOptions.runningUnderAutogen) {
        // Autogen stops before infer but needs to see all definitions
        level = core::StrictLevel::False;
    }

    return level;
}

namespace {

pm_node_t *runPrismRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                              const vector<core::LocOffsets> &commentLocations, const options::Printers &print,
                              core::MutableContext &ctx, const parser::Prism::Parser &parser);

ast::ExpressionPtr fetchTreeFromCache(core::GlobalState &gs, core::FileRef fref, core::File &file,
                                      const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return nullptr;
    }

    if (fref.id() >= gs.filesUsed()) {
        prodCounterInc("types.input.files.kvstore.unindexed");
        return nullptr;
    }

    string fileHashKey = core::serialize::Serializer::fileKey(file);
    auto maybeCached = kvstore->read(fileHashKey);
    if (maybeCached.data == nullptr) {
        prodCounterInc("types.input.files.kvstore.miss");
        return nullptr;
    }

    prodCounterInc("types.input.files.kvstore.hit");
    return core::serialize::Serializer::loadTree(gs, file, maybeCached.data);
}

parser::ParseResult runParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print,
                              bool traceLexer, bool traceParser) {
    Timer timeit(gs.tracer(), "runParser", {{"file", string(file.data(gs).path())}});
    parser::ParseResult result;
    {
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        auto indentationAware = false;               // Don't start in indentation-aware error recovery mode
        auto collectComments = gs.cacheSensitiveOptions.rbsEnabled; // Collect comments for RBS signature translation
        auto settings = parser::Parser::Settings{traceLexer, traceParser, indentationAware, collectComments};
        result = parser::Parser::run(gs, file, settings);
    }

    if (print.ParseTree.enabled) {
        print.ParseTree.fmt("{}\n", result.tree->toStringWithTabs(gs, 0));
    }
    if (print.ParseTreeJson.enabled) {
        print.ParseTreeJson.fmt("{}\n", result.tree->toJSON(gs, 0));
    }
    if (print.ParseTreeJsonWithLocs.enabled) {
        print.ParseTreeJson.fmt("{}\n", result.tree->toJSONWithLocs(gs, file, 0));
    }
    if (print.ParseTreeWhitequark.enabled) {
        print.ParseTreeWhitequark.fmt("{}\n", result.tree->toWhitequark(gs, 0));
    }
    return result;
}

parser::ParseResult runPrismParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print,
                                   const options::Options &opts, bool preserveConcreteSyntax = false) {
    Timer timeit(gs.tracer(), "runParser", {{"file", string(file.data(gs).path())}});

    parser::ParseResult parseResult;
    {
        core::MutableContext ctx(gs, core::Symbols::root(), file);
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        // The RBS rewriter produces plain Whitequark nodes and not `NodeWithExpr` which causes errors in
        // `PrismDesugar.cc`. For now, disable all direct translation, and fallback to `Desugar.cc`.
        auto source = file.data(ctx).source();
        parser::Prism::Parser parser{source};
        bool collectComments = gs.cacheSensitiveOptions.rbsEnabled;
        parser::Prism::ParseResult prismResult = parser.parseWithoutTranslation(collectComments);

        if (opts.stopAfterPhase == options::Phase::PARSER) {
            return parser::ParseResult{nullptr, prismResult.getCommentLocations()};
        }

        auto node = prismResult.getRawNodePointer();

        // TODO: Remove `&& false` once RBS rewriter with Prism AST migration is complete
        // https://github.com/sorbet/sorbet/issues/9065
        if (gs.cacheSensitiveOptions.rbsEnabled && false) {
            node = runPrismRBSRewrite(gs, file, node, prismResult.getCommentLocations(), print, ctx, parser);
        }

        bool directlyDesugar = !gs.cacheSensitiveOptions.rbsEnabled;
        auto translatedTree = parser::Prism::Translator(parser, ctx, prismResult.getParseErrors(), directlyDesugar,
                                                        preserveConcreteSyntax)
                                  .translate(node);

        parseResult = parser::ParseResult{move(translatedTree), prismResult.getCommentLocations()};
    }

    if (parseResult.tree) {
        if (print.ParseTree.enabled) {
            print.ParseTree.fmt("{}\n", parseResult.tree->toStringWithTabs(gs, 0));
        }
        if (print.ParseTreeJson.enabled) {
            print.ParseTreeJson.fmt("{}\n", parseResult.tree->toJSON(gs, 0));
        }
        if (print.ParseTreeJsonWithLocs.enabled) {
            print.ParseTreeJson.fmt("{}\n", parseResult.tree->toJSONWithLocs(gs, file, 0));
        }
        if (print.ParseTreeWhitequark.enabled) {
            print.ParseTreeWhitequark.fmt("{}\n", parseResult.tree->toWhitequark(gs, 0));
        }
    }

    return parseResult;
}

unique_ptr<parser::Node> runRBSRewrite(core::GlobalState &gs, core::FileRef file, parser::ParseResult &&parseResult,
                                       const options::Printers &print) {
    auto node = move(parseResult.tree);
    auto commentLocations = move(parseResult.commentLocations);

    if (gs.cacheSensitiveOptions.rbsEnabled) {
        Timer timeit(gs.tracer(), "runRBSRewrite", {{"file", string(file.data(gs).path())}});
        core::MutableContext ctx(gs, core::Symbols::root(), file);
        core::UnfreezeNameTable nameTableAccess(gs);

        auto associator = rbs::CommentsAssociator(ctx, commentLocations);
        auto commentMap = associator.run(node);

        auto sigsRewriter = rbs::SigsRewriter(ctx, commentMap.signaturesForNode);
        node = sigsRewriter.run(move(node));

        auto assertionsRewriter = rbs::AssertionsRewriter(ctx, commentMap.assertionsForNode);
        node = assertionsRewriter.run(move(node));

        if (print.RBSRewriteTree.enabled) {
            print.RBSRewriteTree.fmt("{}\n", node->toStringWithTabs(gs, 0));
        }
    }
    return node;
}

ast::ExpressionPtr runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                              const options::Printers &print, bool preserveConcreteSyntax = false) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", string(file.data(gs).path())}});
    ast::ExpressionPtr ast;
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    {
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        // The RBS rewriter produces plain Whitequark nodes and not `NodeWithExpr` which causes errors in
        // `PrismDesugar.cc`. For now, disable all direct translation, and fallback to `Desugar.cc`.
        auto directlyDesugar = gs.parseWithPrism && !gs.cacheSensitiveOptions.rbsEnabled;
        ast = directlyDesugar ? ast::prismDesugar::node2Tree(ctx, move(parseTree), preserveConcreteSyntax)
                              : ast::desugar::node2Tree(ctx, move(parseTree), preserveConcreteSyntax);
    }
    if (print.DesugarTree.enabled) {
        print.DesugarTree.fmt("{}\n", ast.toStringWithTabs(gs, 0));
    }
    if (print.DesugarTreeRaw.enabled) {
        print.DesugarTreeRaw.fmt("{}\n", ast.showRaw(gs));
    }
    if (print.DesugarTreeRawWithLocs.enabled) {
        print.DesugarTreeRawWithLocs.fmt("{}\n", ast.showRawWithLocs(gs, file));
    }
    return ast;
}

ast::ExpressionPtr runRewriter(core::GlobalState &gs, core::FileRef file, ast::ExpressionPtr ast) {
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    Timer timeit(gs.tracer(), "runRewriter", {{"file", string(file.data(gs).path())}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
    return rewriter::Rewriter::run(ctx, move(ast));
}

ast::ParsedFile runLocalVars(core::GlobalState &gs, ast::ParsedFile tree) {
    core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
    Timer timeit(gs.tracer(), "runLocalVars", {{"file", string(tree.file.data(gs).path())}});
    core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries when resolving duplicate arguments
    return sorbet::local_vars::LocalVars::run(ctx, move(tree));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {ast::MK::EmptyTree(), file};
}

pm_node_t *runPrismRBSRewrite(core::GlobalState &gs, core::FileRef file, pm_node_t *node,
                              const vector<core::LocOffsets> &commentLocations, const options::Printers &print,
                              core::MutableContext &ctx, const parser::Prism::Parser &parser) {
    Timer timeit(gs.tracer(), "runPrismRBSRewrite", {{"file", string(file.data(gs).path())}});

    auto associator = rbs::CommentsAssociatorPrism(ctx, parser, commentLocations);
    auto commentMap = associator.run(node);

    auto sigsRewriter = rbs::SigsRewriterPrism(ctx, parser, commentMap.signaturesForNode);
    node = sigsRewriter.run(node);

    auto assertionsRewriter = rbs::AssertionsRewriterPrism(ctx, commentMap.assertionsForNode);
    node = assertionsRewriter.run(node);

    if (print.RBSRewriteTree.enabled) {
        print.RBSRewriteTree.fmt("{}\n", parser.prettyPrint(node));
    }

    return node;
}

} // namespace

ast::ExpressionPtr desugarOne(const options::Options &opts, core::GlobalState &gs, core::FileRef file,
                              bool preserveConcreteSyntax) {
    auto &print = opts.print;

    Timer timeit(gs.tracer(), "desugarOne", {{"file", string(file.data(gs).path())}});
    try {
        if (file.data(gs).strictLevel == core::StrictLevel::Ignore) {
            return ast::MK::EmptyTree();
        }
        auto parseResult = runParser(gs, file, print, opts.traceLexer, opts.traceParser);

        auto parseTree = runRBSRewrite(gs, file, move(parseResult), print);

        return runDesugar(gs, file, move(parseTree), print, preserveConcreteSyntax);
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception desugaring file: `{}` (backtrace is above)", file.data(gs).path());
        }
        return ast::MK::EmptyTree();
    }
}

ast::ParsedFile indexOne(const options::Options &opts, core::GlobalState &lgs, core::FileRef file,
                         ast::ExpressionPtr tree) {
    auto &print = opts.print;
    auto parser = opts.cacheSensitiveOptions.usePrismParser ? options::Parser::PRISM : options::Parser::ORIGINAL;

    ast::ParsedFile rewritten{nullptr, file};
    rewritten.setCached(tree != nullptr);

    Timer timeit(lgs.tracer(), "indexOne", {{"file", string(file.data(lgs).path())}});
    try {
        if (!rewritten.cached()) {
            // tree isn't cached. Need to start from parser
            if (file.data(lgs).strictLevel == core::StrictLevel::Ignore) {
                return emptyParsedFile(file);
            }

            unique_ptr<parser::Node> parseTree;
            switch (parser) {
                case options::Parser::ORIGINAL: {
                    auto parseResult = runParser(lgs, file, print, opts.traceLexer, opts.traceParser);
                    if (opts.stopAfterPhase == options::Phase::PARSER) {
                        return emptyParsedFile(file);
                    }

                    parseTree = runRBSRewrite(lgs, file, move(parseResult), print);

                    break;
                }
                case options::Parser::PRISM: {
                    auto parseResult = runPrismParser(lgs, file, print, opts);

                    // parseResult is null if runPrismParser stopped after an intermediate phase
                    if (parseResult.tree == nullptr) {
                        return emptyParsedFile(file);
                    }

                    // TODO: Remove this check once runPrismRBSRewrite is no longer no-oped inside of runPrismParser
                    // https://github.com/sorbet/sorbet/issues/9065
                    parseTree = runRBSRewrite(lgs, file, move(parseResult), print);

                    break;
                }
            }

            tree = runDesugar(lgs, file, move(parseTree), print);
            if (opts.stopAfterPhase == options::Phase::DESUGARER) {
                return emptyParsedFile(file);
            }

            tree = runRewriter(lgs, file, move(tree));
            if (print.RewriterTree.enabled) {
                print.RewriterTree.fmt("{}\n", tree.toStringWithTabs(lgs, 0));
            }
            if (print.RewriterTreeRaw.enabled) {
                print.RewriterTreeRaw.fmt("{}\n", tree.showRaw(lgs));
            }

            tree = runLocalVars(lgs, ast::ParsedFile{move(tree), file}).tree;
            if (opts.stopAfterPhase == options::Phase::LOCAL_VARS) {
                return emptyParsedFile(file);
            }
        }
        if (print.IndexTree.enabled) {
            print.IndexTree.fmt("{}\n", tree.toStringWithTabs(lgs, 0));
        }
        if (print.IndexTreeRaw.enabled) {
            print.IndexTreeRaw.fmt("{}\n", tree.showRaw(lgs));
        }
        if (opts.stopAfterPhase == options::Phase::REWRITER) {
            return emptyParsedFile(file);
        }

        rewritten.tree = move(tree);
        return rewritten;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = lgs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
            e.setHeader("Exception parsing file: `{}` (backtrace is above)", file.data(lgs).path());
        }
        return emptyParsedFile(file);
    }
}

namespace {

void incrementStrictLevelCounter(core::StrictLevel level) {
    switch (level) {
        case core::StrictLevel::None:
            Exception::raise("Should never attempt to increment StrictLevel::None");
            break;
        case core::StrictLevel::Ignore:
            prodCounterInc("types.input.files.sigil.ignore");
            break;
        case core::StrictLevel::Internal:
            Exception::raise("Should never attempt to increment StrictLevel::Internal");
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
            Exception::raise("Should never attempt to increment StrictLevel::Max");
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
ast::ExpressionPtr readFileWithStrictnessOverrides(core::GlobalState &gs, core::FileRef file,
                                                   const options::Options &opts,
                                                   const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    switch (file.dataAllowingUnsafe(gs).sourceType) {
        case core::File::Type::NotYetRead: {
            string fileName{file.dataAllowingUnsafe(gs).path()};
            Timer timeit(gs.tracer(), "readFileWithStrictnessOverrides", {{"file", fileName}});
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
            if (core::File::isRBIPath(fileName)) {
                counterAdd("types.input.rbi.bytes", src.size());
                counterInc("types.input.rbi.files");
            }

            gs.replaceFile(file, make_shared<core::File>(move(fileName), move(src), core::File::Type::Normal));

            if constexpr (enable_counters) {
                counterAdd("types.input.lines", file.data(gs).lineCount());
            }

            if (!fileFound) {
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::FileNotFound)) {
                    e.setHeader("File Not Found");
                }
            }

            break;
        }
        case core::File::Type::Normal: {
            // If we load successfully from the cache then there weren't any indexing errors, and otherwise we'll set
            // this flag during indexing if necessary.
            file.data(gs).setHasIndexErrors(false);

            break;
        }
        case core::File::Type::PayloadGeneration:
        case core::File::Type::Payload:
        case core::File::Type::TombStone:
            return nullptr;
    }

    auto &fileData = file.data(gs);

    // Returns nullptr if tree is not in cache.
    auto ast = fetchTreeFromCache(gs, file, fileData, kvstore);

    if (!opts.storeState.empty()) {
        fileData.sourceType = core::File::Type::PayloadGeneration;
    }

    auto level = decideStrictLevel(gs, file, opts);
    fileData.strictLevel = level;
    incrementStrictLevelCounter(level);
    return ast;
}

struct IndexResult {
    unique_ptr<core::GlobalState> gs;
    vector<ast::ParsedFile> trees;

    // The number of trees that were processed by the thread that produced this result. This can be greater than
    // `trees.size()` when cancelation happens, as we skip trees to save time at that point. This value must be used in
    // place of `trees.size()` anywhere that we're accounting for the number of trees processed by indexing, otherwise
    // we risk starving the main thread which expects to read one result for every tree processed.
    int numTreesProcessed = 0;
};

struct IndexThreadResultPack {
    CounterState counters;
    IndexResult res;
};

struct IndexSubstitutionJob {
    // Not necessary for substitution, but passing this through to the worker means it's freed in that thread, instead
    // of serially in the main thread.
    unique_ptr<core::GlobalState> threadGs;

    optional<core::NameSubstitution> subst;
    vector<ast::ParsedFile> trees;

    // Please see the comment on `IndexResult::numTreesProcessed` for a more thorough description about why this might
    // be greater than `trees.size()`.
    int numTreesProcessed = 0;

    IndexSubstitutionJob() {}

    IndexSubstitutionJob(core::GlobalState &to, IndexResult res)
        : threadGs{std::move(res.gs)}, subst{}, trees{std::move(res.trees)}, numTreesProcessed{res.numTreesProcessed} {
        if (absl::c_any_of(this->trees, [](auto &parsed) { return !parsed.cached(); })) {
            this->subst.emplace(*this->threadGs, to);
        }
    }

    IndexSubstitutionJob(IndexSubstitutionJob &&other) = default;
    IndexSubstitutionJob &operator=(IndexSubstitutionJob &&other) = default;
};

ast::ParsedFilesOrCancelled mergeIndexResults(core::GlobalState &cgs, const options::Options &opts,
                                              shared_ptr<BlockingBoundedQueue<IndexThreadResultPack>> input,
                                              WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore,
                                              bool cancelable) {
    ProgressIndicator progress(opts.showProgress, "Indexing", input->bound);

    auto batchq = make_shared<ConcurrentBoundedQueue<IndexSubstitutionJob>>(input->bound);
    vector<ast::ParsedFile> ret;
    size_t totalNumTrees = 0;

    {
        Timer timeit(cgs.tracer(), "mergeGlobalStates");
        IndexThreadResultPack threadResult;
        for (auto result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs.tracer());
             !result.done(); result = input->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), cgs.tracer())) {
            if (result.gotItem()) {
                counterConsume(move(threadResult.counters));
                auto numTrees = threadResult.res.numTreesProcessed;
                batchq->push(IndexSubstitutionJob{cgs, std::move(threadResult.res)}, numTrees);
                totalNumTrees += numTrees;
            }
        }
    }

    {
        Timer timeit(cgs.tracer(), "substituteTrees");
        auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(batchq->bound);

        workers.multiplexJob(
            "substituteTrees", [&cgs = as_const(cgs), &logger = cgs.tracer(), batchq, resultq, cancelable]() {
                Timer timeit(logger, "substituteTreesWorker");
                IndexSubstitutionJob job;
                int numTreesProcessed = 0;
                vector<ast::ParsedFile> trees;
                for (auto result = batchq->try_pop(job); !result.done(); result = batchq->try_pop(job)) {
                    if (result.gotItem()) {
                        // Unconditionally update the total to avoid starving the consumer thread
                        numTreesProcessed += job.numTreesProcessed;

                        // If the slow path has been cancelled, skip substitution to handle the tree dropping in once
                        // place.
                        if (cancelable && cgs.epochManager->wasTypecheckingCanceled()) {
                            continue;
                        }

                        if (job.subst.has_value()) {
                            for (auto &tree : job.trees) {
                                if (!tree.cached()) {
                                    core::Context ctx(cgs, core::Symbols::root(), tree.file);
                                    tree = ast::Substitute::run(ctx, *job.subst, move(tree));
                                }
                            }
                        }

                        trees.insert(trees.end(), std::make_move_iterator(job.trees.begin()),
                                     std::make_move_iterator(job.trees.end()));
                    }
                }

                if (numTreesProcessed > 0) {
                    resultq->push(std::move(trees), numTreesProcessed);
                }
            });

        ret.reserve(totalNumTrees);
        vector<ast::ParsedFile> trees;
        for (auto result = resultq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), cgs.tracer()); !result.done();
             result = resultq->wait_pop_timed(trees, WorkerPool::BLOCK_INTERVAL(), cgs.tracer())) {
            if (result.gotItem()) {
                ret.insert(ret.end(), std::make_move_iterator(trees.begin()), std::make_move_iterator(trees.end()));
                progress.reportProgress(resultq->doneEstimate());
            }
        }
    }

    if (cancelable && cgs.epochManager->wasTypecheckingCanceled()) {
        return ast::ParsedFilesOrCancelled::cancel(std::move(ret), workers);
    }

    return ret;
}

ast::ParsedFilesOrCancelled indexSuppliedFiles(core::GlobalState &baseGs, absl::Span<const core::FileRef> files,
                                               const options::Options &opts, WorkerPool &workers,
                                               const unique_ptr<const OwnedKeyValueStore> &kvstore, bool cancelable) {
    auto resultq = make_shared<BlockingBoundedQueue<IndexThreadResultPack>>(files.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(files.size());
    for (auto file : files) {
        fileq->push(move(file), 1);
    }

    shared_ptr<const core::GlobalState> emptyGs = baseGs.copyForIndexThread(
        opts.cacheSensitiveOptions.sorbetPackages, opts.extraPackageFilesDirectoryUnderscorePrefixes,
        opts.extraPackageFilesDirectorySlashDeprecatedPrefixes, opts.extraPackageFilesDirectorySlashPrefixes,
        opts.packageSkipRBIExportEnforcementDirs, opts.allowRelaxedPackagerChecksFor, opts.packagerLayers,
        opts.sorbetPackagesHint, opts.genPackages);

    workers.multiplexJob("indexSuppliedFiles", [emptyGs, &opts, fileq, resultq, &kvstore, cancelable]() {
        Timer timeit(emptyGs->tracer(), "indexSuppliedFilesWorker");

        // clone the empty global state to avoid manually re-entering everything, and copy the base filetable so that
        // file sources are available.
        auto localGs = emptyGs->copyForIndexThread(
            opts.cacheSensitiveOptions.sorbetPackages, opts.extraPackageFilesDirectoryUnderscorePrefixes,
            opts.extraPackageFilesDirectorySlashDeprecatedPrefixes, opts.extraPackageFilesDirectorySlashPrefixes,
            opts.packageSkipRBIExportEnforcementDirs, opts.allowRelaxedPackagerChecksFor, opts.packagerLayers,
            opts.sorbetPackagesHint, opts.genPackages);
        auto &epochManager = *localGs->epochManager;

        IndexThreadResultPack threadResult;

        {
            core::FileRef job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    // Increment the count even if we're cancelled to ensure that we indicate downstream that all inputs
                    // have been processed.
                    threadResult.res.numTreesProcessed++;

                    // Drain the queue if the slow path gets canceled.
                    if (cancelable && epochManager.wasTypecheckingCanceled()) {
                        continue;
                    }
                    core::FileRef file = job;
                    auto cachedTree = readFileWithStrictnessOverrides(*localGs, file, opts, kvstore);
                    auto parsedFile = indexOne(opts, *localGs, file, move(cachedTree));
                    threadResult.res.trees.emplace_back(move(parsedFile));
                }
            }
        }

        if (threadResult.res.numTreesProcessed > 0) {
            threadResult.counters = getAndClearThreadCounters();
            threadResult.res.gs = move(localGs);
            resultq->push(move(threadResult), threadResult.res.numTreesProcessed);
        }
    });

    return mergeIndexResults(baseGs, opts, resultq, workers, kvstore, cancelable);
}

} // namespace

ast::ParsedFilesOrCancelled index(core::GlobalState &gs, absl::Span<const core::FileRef> files,
                                  const options::Options &opts, WorkerPool &workers,
                                  const unique_ptr<const OwnedKeyValueStore> &kvstore, bool cancelable) {
    Timer timeit(gs.tracer(), "index");
    vector<ast::ParsedFile> empty;

    if (opts.stopAfterPhase == options::Phase::INIT) {
        return empty;
    }

    gs.sanityCheck();

    if (files.size() < 3) {
        // Run singlethreaded if only using 2 files
        vector<ast::ParsedFile> parsed;
        parsed.reserve(files.size());
        for (auto file : files) {
            auto tree = readFileWithStrictnessOverrides(gs, file, opts, kvstore);
            auto parsedFile = indexOne(opts, gs, file, move(tree));
            parsed.emplace_back(move(parsedFile));
        }

        if (cancelable && gs.epochManager->wasTypecheckingCanceled()) {
            return ast::ParsedFilesOrCancelled::cancel(std::move(parsed), workers);
        }

        // TODO(jez) Do we want this fast_sort here? Is it redundant?
        fast_sort(parsed, [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
        ENFORCE(files.size() == parsed.size());
        return parsed;
    } else {
        auto ret = indexSuppliedFiles(gs, files, opts, workers, kvstore, cancelable);
        if (ret.hasResult()) {
            // TODO(jez) Do we want this fast_sort here? Is it redundant?
            fast_sort(ret.result(), [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
        }
        return ret;
    }
}

// ----- packager -------------------------------------------------------------

size_t partitionPackageFiles(const core::GlobalState &gs, absl::Span<core::FileRef> inputFiles) {
    ENFORCE(gs.packageDB().enabled());
    // c_partition does not maintain relative ordering of the elements, which means that
    // the sort order of the file paths is not preserved.
    //
    // index doesn't depend on this order, because it is already indexes files in
    // parallel and sorts the resulting parsed files at the end. For that reason, I've
    // chosen not to use stable_partition here.
    auto packageFilesEnd = absl::c_partition(inputFiles, [&](auto f) { return f.isPackage(gs); });
    auto numPackageFiles = distance(inputFiles.begin(), packageFilesEnd);
    return numPackageFiles;
}

void unpartitionPackageFiles(vector<ast::ParsedFile> &packageFiles, vector<ast::ParsedFile> &&nonPackageFiles) {
    if (packageFiles.empty()) {
        // Performance optimization--if it's already empty, no need to move one-by-one
        packageFiles = move(nonPackageFiles);
    } else {
        // In this case, all the __package.rb files will have been sorted before non-__package.rb files,
        // and within each subsequence, the parsed files will be sorted (pipeline::index sorts its result)
        packageFiles.reserve(packageFiles.size() + nonPackageFiles.size());
        absl::c_move(nonPackageFiles, back_inserter(packageFiles));
    }
}

vector<CondensationStratumInfo> computePackageStrata(const core::GlobalState &gs, vector<ast::ParsedFile> &packageFiles,
                                                     absl::Span<core::FileRef> sourceFiles,
                                                     const options::Options &opts) {
    Timer timeit(gs.tracer(), "computePackageStrata");

    if (!opts.packageDirected) {
        return vector{CondensationStratumInfo{absl::MakeSpan(packageFiles), sourceFiles}};
    }

    auto &db = gs.packageDB();

    auto numPackageFiles = packageFiles.size();

    // We move all the package files into a map, so that it's easy to go from package name to package file while
    // processing the strata of the traversal.
    UnorderedMap<core::packages::MangledName, ast::ParsedFile> packagesToPackageRb;
    for (auto &ast : packageFiles) {
        auto pkgName = db.getPackageNameForFile(ast.file);
        ENFORCE(pkgName.exists());

        packagesToPackageRb[pkgName] = move(ast);
    }

    // We'll generate non-test variants of each package, so we need to double the storage of this vector.
    packageFiles.clear();
    packageFiles.reserve(numPackageFiles * 2);

    auto traversal = db.condensation().computeTraversal(gs);
    ENFORCE(!traversal.strata.empty());

    vector<uint32_t> fileToStratum(gs.filesUsed());
    {
        Timer timeit(gs.tracer(), "computePackageStrata.stratumMapping");

        auto stratumMapping = traversal.buildStratumMapping(gs);

        int ix = 0;
        for (auto &file : gs.getFiles().subspan(1)) {
            ++ix;
            core::FileRef fref(ix);
            auto pkgName = db.getPackageNameForFile(fref);
            if (!pkgName.exists()) {
                // This is a file with no package, so we unconditionally put it in the first stratum. This means that
                // it'll be checked at the same time as the first stratum of packaged code.
                fileToStratum[ix] = 0;
                continue;
            }

            ENFORCE(stratumMapping.find(pkgName) != stratumMapping.end(),
                    "All packages must be present in the condensation graph");
            auto &info = stratumMapping[pkgName];
            if (file->isPackagedTest() || file->isPackagedTestHelper()) {
                fileToStratum[ix] = info.testStratum;
            } else {
                fileToStratum[ix] = info.applicationStratum;
            }
        }

        fast_sort(sourceFiles, [&fileToStratum = as_const(fileToStratum)](core::FileRef l, core::FileRef r) -> bool {
            auto lid = l.id();
            auto rid = r.id();
            auto lStratum = fileToStratum[lid];
            auto rStratum = fileToStratum[rid];
            return std::tie(lStratum, lid) < std::tie(rStratum, rid);
        });
    }

    // Reserve enough space for all the strata of the condensation traversal, plus one more for unpackaged code.
    auto maxStrata = traversal.strata.size() + 1;

    vector<CondensationStratumInfo> result;
    result.reserve(maxStrata);

    auto sourceSpan = sourceFiles;

    int currentStratum = -1;
    for (auto &stratum : traversal.strata) {
        ++currentStratum;

        auto &resultStratum = result.emplace_back();

        {
            auto start = packageFiles.size();
            for (auto &scc : stratum) {
                if (scc.isTest) {
                    for (auto member : scc.members) {
                        packageFiles.emplace_back(std::move(packagesToPackageRb[member]));
                    }
                } else {
                    // When processing the application source of a package, we need to make a copy of the package file,
                    // and edit out any reference to test symbols.
                    for (auto member : scc.members) {
                        const auto &package = packagesToPackageRb[member];
                        packageFiles.emplace_back(packager::Packager::copyPackageWithoutTestExports(gs, package));
                    }
                }
            }

            auto len = packageFiles.size() - start;
            ENFORCE(len > 0);
            resultStratum.packageFiles = absl::MakeSpan(packageFiles).subspan(start, len);
        }

        {
            auto it = absl::c_find_if(sourceSpan, [currentStratum, &fileToStratum = as_const(fileToStratum)](
                                                      auto file) { return fileToStratum[file.id()] > currentStratum; });
            auto len = std::distance(sourceSpan.begin(), it);
            resultStratum.sourceFiles = sourceSpan.subspan(0, len);
            sourceSpan = sourceSpan.subspan(len);
        }
    }

    ENFORCE(sourceSpan.empty());
    ENFORCE(!result.empty());
    ENFORCE(result.size() <= maxStrata);

    return result;
}

// packager intentionally runs outside of rewriter so that its output does not get cached.
void buildPackageDB(core::GlobalState &gs, absl::Span<ast::ParsedFile> what, absl::Span<core::FileRef> nonPackageFiles,
                    const options::Options &opts, WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    if (!opts.cacheSensitiveOptions.sorbetPackages) {
        return;
    }

    try {
        packager::Packager::buildPackageDB(gs, workers, what, nonPackageFiles);
        if (opts.print.Packager.enabled) {
            for (auto &f : what) {
                opts.print.Packager.fmt("# -- {} --\n", f.file.data(gs).path());
                opts.print.Packager.fmt("{}\n", f.tree.toStringWithTabs(gs, 0));
            }
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception packaging (backtrace is above)");
        }
    }
#endif
}

// packager intentionally runs outside of rewriter so that its output does not get cached.
void validatePackagedFiles(core::GlobalState &gs, absl::Span<ast::ParsedFile> what, const options::Options &opts,
                           WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    if (!opts.cacheSensitiveOptions.sorbetPackages) {
        return;
    }

    try {
        packager::Packager::validatePackagedFiles(gs, workers, what);
        if (opts.print.Packager.enabled) {
            for (auto &f : what) {
                opts.print.Packager.fmt("# -- {} --\n", f.file.data(gs).path());
                opts.print.Packager.fmt("{}\n", f.tree.toStringWithTabs(gs, 0));
            }
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception packaging (backtrace is above)");
        }
    }
#endif
}

// ----- namer & resolver -----------------------------------------------------

namespace {

class GatherUnresolvedConstantsWalk {
public:
    vector<string> unresolvedConstants;

    void postTransformConstantLit(core::MutableContext ctx, const ast::ConstantLit &tree) {
        auto unresolvedPath = tree.fullUnresolvedPath(ctx);
        if (unresolvedPath.has_value()) {
            unresolvedConstants.emplace_back(fmt::format(
                "{}::{}", unresolvedPath->first != core::Symbols::root() ? unresolvedPath->first.show(ctx) : "",
                fmt::map_join(unresolvedPath->second, "::", [&](const auto &el) -> string { return el.show(ctx); })));
        }
    }

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &classDef) {
        if (classDef.kind == ast::ClassDef::Kind::Class && !classDef.ancestors.empty()) {
            auto lit = ast::cast_tree<ast::ConstantLit>(classDef.ancestors.front());
            auto unresolvedPath = lit->fullUnresolvedPath(ctx);
            if (unresolvedPath.has_value()) {
                unresolvedConstants.emplace_back(fmt::format(
                    "{}::{}", unresolvedPath->first != core::Symbols::root() ? unresolvedPath->first.show(ctx) : "",
                    fmt::map_join(unresolvedPath->second,
                                  "::", [&](const auto &el) -> string { return el.show(ctx); })));
            }
        }
    }
};

vector<ast::ParsedFile> printMissingConstants(core::GlobalState &gs, const options::Options &opts,
                                              vector<ast::ParsedFile> what) {
    Timer timeit(gs.tracer(), "printMissingConstants");
    GatherUnresolvedConstantsWalk walk;
    for (auto &resolved : what) {
        core::MutableContext ctx(gs, core::Symbols::root(), resolved.file);
        ast::ConstTreeWalk::apply(ctx, walk, resolved.tree);
    }
    auto &missing = walk.unresolvedConstants;
    fast_sort(missing);
    missing.erase(unique(missing.begin(), missing.end()), missing.end());

    opts.print.MissingConstants.fmt("{}\n", fmt::join(missing, "\n"));
    return what;
}

class DefinitionLinesDenylistEnforcer {
private:
    const core::FileRef file;
    const int prohibitedLinesStart;
    const int prohibitedLinesEnd;

    bool isAllowListed(core::Context ctx, core::SymbolRef sym) {
        return sym.name(ctx) == core::Names::staticInit() || sym.name(ctx) == core::Names::Constants::Root() ||
               sym.name(ctx) == core::Names::unresolvedAncestors();
    }

    void checkLoc(core::Context ctx, core::Loc loc) {
        auto detailStart = core::Loc::pos2Detail(file.data(ctx), loc.beginPos());
        auto detailEnd = core::Loc::pos2Detail(file.data(ctx), loc.endPos());
        ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
    }

    void checkSym(core::Context ctx, core::SymbolRef sym) {
        if (isAllowListed(ctx, sym)) {
            return;
        }
        checkLoc(ctx, sym.loc(ctx));
    }

public:
    DefinitionLinesDenylistEnforcer(core::FileRef file, int prohibitedLinesStart, int prohibitedLinesEnd)
        : file(file), prohibitedLinesStart(prohibitedLinesStart), prohibitedLinesEnd(prohibitedLinesEnd) {
        // Can be equal if file was empty.
        ENFORCE(prohibitedLinesStart <= prohibitedLinesEnd);
        ENFORCE(file.exists());
    };

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &tree) {
        checkSym(ctx, tree.symbol);
    }
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &tree) {
        checkSym(ctx, tree.symbol);
    }
};

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd) {
    DefinitionLinesDenylistEnforcer enforcer(what.file, prohibitedLinesStart, prohibitedLinesEnd);
    ast::ConstTreeWalk::apply(core::Context(gs, core::Symbols::root(), what.file), enforcer, what.tree);
    return what;
}

} // namespace

[[nodiscard]] bool name(core::GlobalState &gs, absl::Span<ast::ParsedFile> what, const options::Options &opts,
                        WorkerPool &workers, core::FoundDefHashes *foundHashes) {
    Timer timeit(gs.tracer(), "name");
    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
    auto packagesAccess = gs.unfreezePackages();
    bool canceled = false;
    try {
        canceled = namer::Namer::run(gs, what, workers, foundHashes);
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception naming (backtrace is above)");
        }
    }

    if (!canceled) {
        for (auto &named : what) {
            if (opts.print.NameTree.enabled) {
                opts.print.NameTree.fmt("{}\n", named.tree.toStringWithTabs(gs, 0));
            }
            if (opts.print.NameTreeRaw.enabled) {
                opts.print.NameTreeRaw.fmt("{}\n", named.tree.showRaw(gs));
            }
        }
    }

    return canceled;
}

ast::ParsedFilesOrCancelled resolve(core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
                                    WorkerPool &workers) {
    try {
        if (opts.stopAfterPhase != options::Phase::NAMER) {
            ProgressIndicator namingProgress(opts.showProgress, "Resolving", 1);
            {
                Timer timeit(gs.tracer(), "resolving");
                core::UnfreezeNameTable nameTableAccess(gs);     // Resolver::defineAttr
                core::UnfreezeSymbolTable symbolTableAccess(gs); // enters stubs
                auto maybeResult = resolver::Resolver::run(gs, move(what), workers);
                if (!maybeResult.hasResult()) {
                    return maybeResult;
                }
                what = move(maybeResult.result());
            }

#ifndef SORBET_REALMAIN_MIN
            if (opts.cacheSensitiveOptions.sorbetPackages) {
                Timer timeit(gs.tracer(), "visibility_checker");
                what = packager::VisibilityChecker::run(gs, workers, std::move(what));
            }
#endif

            if (opts.stressIncrementalResolver) {
                auto symbolsBefore = gs.symbolsUsedTotal();
                for (auto &f : what) {
                    // Shift contents of file past current file's EOF, re-run incrementalResolve, assert that no
                    // locations appear before file's old EOF.
                    const int prohibitedLines = f.file.data(gs).source().size();
                    auto newSource = fmt::format("{}\n{}", string(prohibitedLines, '\n'), f.file.data(gs).source());
                    auto newFile = make_shared<core::File>(string(f.file.data(gs).path()), move(newSource),
                                                           f.file.data(gs).sourceType);
                    gs.replaceFile(f.file, move(newFile));
                    f.file.data(gs).strictLevel = decideStrictLevel(gs, f.file, opts);
                    auto reIndexed = indexOne(opts, gs, f.file);
                    vector<ast::ParsedFile> toBeReResolved;
                    toBeReResolved.emplace_back(move(reIndexed));
                    // We don't compute file hashes when running for incrementalResolve.
                    auto foundHashesForFiles = nullopt;
                    auto reresolved =
                        pipeline::incrementalResolve(gs, move(toBeReResolved), foundHashesForFiles, opts, workers);
                    ENFORCE(reresolved.size() == 1);
                    f = checkNoDefinitionsInsideProhibitedLines(gs, move(reresolved[0]), 0, prohibitedLines);
                }
                ENFORCE(symbolsBefore == gs.symbolsUsedTotal(),
                        "Stressing the incremental resolver should not add any new symbols");
            }
        }
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (auto e = gs.beginError(sorbet::core::Loc::none(), core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    if (opts.print.ResolveTree.enabled || opts.print.ResolveTreeRaw.enabled) {
        for (auto &resolved : what) {
            if (opts.print.ResolveTree.enabled) {
                opts.print.ResolveTree.fmt("{}\n", resolved.tree.toString(gs));
            }
            if (opts.print.ResolveTreeRaw.enabled) {
                opts.print.ResolveTreeRaw.fmt("{}\n", resolved.tree.showRaw(gs));
            }
        }
    }

    if (opts.print.SymbolTable.enabled) {
        opts.print.SymbolTable.fmt("{}\n", gs.toString());
    }
    if (opts.print.SymbolTableRaw.enabled) {
        opts.print.SymbolTableRaw.fmt("{}\n", gs.showRaw());
    }

#ifndef SORBET_REALMAIN_MIN
    if (opts.print.SymbolTableJson.enabled) {
        auto root = core::Proto::toProto(gs, core::Symbols::root(), false);
        if (opts.print.SymbolTableJson.outputPath.empty()) {
            core::Proto::toJSON(root, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(root, buf);
            opts.print.SymbolTableJson.print(buf.str());
        }
    }
    if (opts.print.SymbolTableProto.enabled) {
        auto root = core::Proto::toProto(gs, core::Symbols::root(), false);
        if (opts.print.SymbolTableProto.outputPath.empty()) {
            root.SerializeToOstream(&cout);
        } else {
            string buf;
            root.SerializeToString(&buf);
            opts.print.SymbolTableProto.print(buf);
        }
    }
    if (opts.print.SymbolTableMessagePack.enabled) {
        auto root = core::Proto::toProto(gs, core::Symbols::root(), false);
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
        auto root = core::Proto::toProto(gs, core::Symbols::root(), true);
        if (opts.print.SymbolTableJson.outputPath.empty()) {
            core::Proto::toJSON(root, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(root, buf);
            opts.print.SymbolTableJson.print(buf.str());
        }
    }
    if (opts.print.SymbolTableFullProto.enabled) {
        auto root = core::Proto::toProto(gs, core::Symbols::root(), true);
        if (opts.print.SymbolTableFullProto.outputPath.empty()) {
            root.SerializeToOstream(&cout);
        } else {
            string buf;
            root.SerializeToString(&buf);
            opts.print.SymbolTableFullProto.print(buf);
        }
    }
    if (opts.print.SymbolTableFullMessagePack.enabled) {
        auto root = core::Proto::toProto(gs, core::Symbols::root(), true);
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
        opts.print.SymbolTableFull.fmt("{}\n", gs.toStringFull());
    }
    if (opts.print.SymbolTableFullRaw.enabled) {
        opts.print.SymbolTableFullRaw.fmt("{}\n", gs.showRawFull());
    }

    if (opts.print.MissingConstants.enabled) {
        what = printMissingConstants(gs, opts, move(what));
    }

    return ast::ParsedFilesOrCancelled(move(what));
}

ast::ParsedFilesOrCancelled nameAndResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                                           const options::Options &opts, WorkerPool &workers,
                                           core::FoundDefHashes *foundHashes) {
    auto canceled = name(gs, absl::Span<ast::ParsedFile>(what), opts, workers, foundHashes);
    if (canceled) {
        return ast::ParsedFilesOrCancelled::cancel(move(what), workers);
    }

    return resolve(gs, move(what), opts, workers);
}

vector<ast::ParsedFile>
incrementalResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                   optional<UnorderedMap<core::FileRef, shared_ptr<const core::FileHash>>> &&foundHashesForFiles,
                   const options::Options &opts, WorkerPool &workers) {
    try {
        vector<core::ClassOrModuleRef> symbolsToRecompute;
        auto runIncrementalNamer = foundHashesForFiles.has_value() && !foundHashesForFiles->empty();
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);
            auto packages = gs.unfreezePackages();

            auto canceled =
                runIncrementalNamer
                    ? namer::Namer::runIncremental(gs, absl::Span<ast::ParsedFile>(what),
                                                   std::move(foundHashesForFiles.value()), workers, symbolsToRecompute)
                    : namer::Namer::run(gs, absl::Span<ast::ParsedFile>(what), workers, nullptr);

            // Cancellation cannot occur during incremental namer.
            ENFORCE(!canceled);

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::NAMER) {
                return what;
            }
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.cacheSensitiveOptions.sorbetPackages) {
            Timer timeit(gs.tracer(), "incremental_packager");
            // For simplicity, we still call Packager::runIncremental here, even though
            // pipeline::nameAndResolve no longer calls Packager::run.
            //
            // TODO(jez) We may want to revisit this. At the moment, the only thing that
            // runIncremental does is validate that files have the right package prefix. We could
            // split `pipeline::package` into something like "populate the package DB" and "verify
            // the package prefixes" with the later living in `pipeline::nameAndResolve` once again
            // (thus restoring the symmetry).
            what = packager::Packager::runIncremental(gs, move(what), workers);
        }
#endif

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            auto result = resolver::Resolver::runIncremental(gs, move(what), runIncrementalNamer, workers,
                                                             absl::MakeSpan(symbolsToRecompute));
            // incrementalResolve is not cancelable.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::RESOLVER) {
                return what;
            }
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.cacheSensitiveOptions.sorbetPackages) {
            what = packager::VisibilityChecker::run(gs, workers, std::move(what));
        }
#endif

    } catch (SorbetException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

// ----- typecheck ------------------------------------------------------------

namespace {

// Check if a file has the "# compiled: true" sigil
static bool isCompiledFile(core::Context ctx) {
    auto source = ctx.file.data(ctx).source();
    auto searchRegion = source.substr(0, std::min(source.size(), size_t(500)));
    return searchRegion.find("compiled: true") != std::string_view::npos;
}

class CFGCollectorAndTyper {
    const options::Options &opts;

public:
    CFGCollectorAndTyper(const options::Options &opts) : opts(opts){};

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &m = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        const auto &methodData = m.symbol.data(ctx);
        bool isAbstract = methodData->flags.isAbstract;
        bool isCompiled = isCompiledFile(ctx);

        // For abstract methods in compiled files, we build CFG and run semantic extensions
        // but skip inference (to avoid type errors from the synthetic super call).
        // For all other cases, use the normal willRun check.
        if (!isAbstract && !infer::Inference::willRun(ctx, m.declLoc, m.symbol)) {
            return;
        }
        if (isAbstract && !isCompiled) {
            return;
        }

        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m.symbol), m);

        if (opts.stopAfterPhase != options::Phase::CFG) {
            // Skip inference for abstract methods (to avoid type errors from synthetic super call)
            if (!isAbstract) {
                cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
            }
            if (cfg) {
                for (auto &extension : ctx.state.semanticExtensions) {
                    extension->typecheck(ctx, ctx.file, *cfg, m);
                }
            }
        }
        if (print.CFG.enabled) {
            print.CFG.fmt("{}\n\n", cfg->toString(ctx));
        }
        if (print.CFGText.enabled) {
            print.CFG.fmt("{}\n\n", cfg->toTextualString(ctx));
        }
        if (print.CFGRaw.enabled) {
            print.CFGRaw.fmt("{}\n\n", cfg->showRaw(ctx));
        }
    }
};

void typecheckOne(core::Context ctx, ast::ParsedFile resolved, const options::Options &opts,
                  bool intentionallyLeakASTs) {
    core::FileRef f = resolved.file;

    if (opts.stopAfterPhase == options::Phase::NAMER) {
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }

    resolved = class_flatten::runOne(ctx, move(resolved));

    definition_validator::runOne(ctx, resolved);

    if (opts.print.FlattenTree.enabled || opts.print.AST.enabled) {
        opts.print.FlattenTree.fmt("{}\n", resolved.tree.toString(ctx));
    }
    if (opts.print.FlattenTreeRaw.enabled || opts.print.ASTRaw.enabled) {
        opts.print.FlattenTreeRaw.fmt("{}\n", resolved.tree.showRaw(ctx));
    }

    if (opts.stopAfterPhase == options::Phase::RESOLVER) {
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }
    if (f.data(ctx).isRBI() && ctx.state.lspQuery.isEmpty()) {
        // If this is an RBI file but isEmpty is not set, we want to run inference just so that we
        // can get hover, completion, and definition requests.
        //
        // There may be type errors in the file (e.g., you don't need `extend T::Sig` to write `sig`
        // in an RBI file), but we already ignore errors produced in service of an LSPQuery.
        if (intentionallyLeakASTs) {
            intentionallyLeakMemory(resolved.tree.release());
        }
        return;
    }

    Timer timeit(ctx.state.tracer(), "typecheckOne", {{"file", string(f.data(ctx).path())}});
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
            ast::ShallowWalk::apply(ctx, collector, resolved.tree);
            if (f.data(ctx).isRBI()) {
                return;
            }
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

    if (intentionallyLeakASTs) {
        intentionallyLeakMemory(resolved.tree.release());
    }
    return;
}

} // namespace

void typecheck(const core::GlobalState &gs, vector<ast::ParsedFile> what, const options::Options &opts,
               WorkerPool &workers, bool cancelable,
               optional<shared_ptr<core::lsp::PreemptionTaskManager>> preemptionManager, bool presorted,
               bool intentionallyLeakASTs) {
    // Unless the error queue had a critical error, only typecheck should flush errors to the client, otherwise we will
    // drop errors in LSP mode.
    ENFORCE(gs.hadCriticalError() || gs.errorQueue->filesFlushedCount == 0);

    const auto &epochManager = *gs.epochManager;
    // Record epoch at start of typechecking before any preemption occurs.
    const uint32_t epoch = epochManager.getStatus().epoch;

    {
        Timer timeit(gs.tracer(), "typecheck");
        if (preemptionManager) {
            // Before kicking off typechecking, check if we need to preempt.
            (*preemptionManager)->tryRunScheduledPreemptionTask(gs);
        }

        auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
        auto outputq = make_shared<BlockingBoundedQueue<core::FileRef>>(what.size());

        if (!presorted) {
            // If files are not already sorted, we want to start typeckecking big files first because it helps with
            // better work distribution
            fast_sort(what, [&](const auto &lhs, const auto &rhs) -> bool {
                return lhs.file.data(gs).source().size() > rhs.file.data(gs).source().size();
            });
        }

        for (auto &resolved : what) {
            fileq->push(move(resolved), 1);
        }

        {
            ProgressIndicator cfgInferProgress(opts.showProgress, "CFG+Inference", what.size());
            workers.multiplexJob("typecheck", [&gs, &opts, epoch, &epochManager, &preemptionManager, fileq, outputq,
                                               cancelable, intentionallyLeakASTs]() {
                ast::ParsedFile job;
                int processedByThread = 0;

                {
                    for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                        if (result.gotItem()) {
                            unique_ptr<absl::ReaderMutexLock> lock;
                            if (preemptionManager) {
                                // [IDE] While held, no preemption tasks can run. Auto-released after each turn of
                                // the loop. Does not starve writers (tryRunScheduledPreemptionTask)
                                // because this call can block once tryRunScheduledPreemptionTask tries to acquire
                                // a (writer) lock.
                                lock = (*preemptionManager)->lockPreemption();
                            }
                            processedByThread++;
                            // [IDE] Only do the work if typechecking hasn't been canceled.
                            const bool isCanceled = cancelable && epochManager.wasTypecheckingCanceled();
                            // [IDE] Also, don't do work if the file has changed under us since we began
                            // typechecking!
                            // TODO(jvilk): epoch is unlikely to overflow, but it is theoretically possible.
                            const bool fileWasChanged = preemptionManager && job.file.data(gs).epoch > epoch;
                            if (!isCanceled && !fileWasChanged && gs.errorQueue->wouldFlushErrorsForFile(job.file)) {
                                core::FileRef file = job.file;
                                try {
                                    core::Context ctx(gs, core::Symbols::root(), file);
                                    typecheckOne(ctx, move(job), opts, intentionallyLeakASTs);
                                } catch (SorbetException &) {
                                    Exception::failInFuzzer();
                                    gs.tracer().error("Exception typing file: {} (backtrace is above)",
                                                      file.data(gs).path());
                                }
                                // Stream out errors
                                outputq->push(file, processedByThread);
                                processedByThread = 0;
                            }
                        }
                    }
                }
                if (processedByThread > 0) {
                    outputq->push(core::FileRef(), processedByThread);
                }
            });

            {
                core::FileRef file;
                for (auto result = outputq->wait_pop_timed(file, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                     !result.done();
                     result = outputq->wait_pop_timed(file, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                    if (result.gotItem()) {
                        if (file.exists()) {
                            gs.errorQueue->flushErrorsForFile(gs, file);
                        }
                    }
                    cfgInferProgress.reportProgress(fileq->doneEstimate());

                    if (preemptionManager) {
                        (*preemptionManager)->tryRunScheduledPreemptionTask(gs);
                    }
                }
                if (cancelable && epochManager.wasTypecheckingCanceled()) {
                    return;
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
        for (auto &extension : gs.semanticExtensions) {
            extension->finishTypecheck(gs);
        }

        // Error queue is re-used across runs, so reset the flush count to ignore files flushed during typecheck.
        gs.errorQueue->filesFlushedCount = 0;

        return;
    }
}

// ----- other ----------------------------------------------------------------

void printFileTable(core::GlobalState &gs, const options::Options &opts,
                    const UnorderedMap<long, long> &untypedUsages) {
#ifndef SORBET_REALMAIN_MIN
    if (opts.print.FileTableProto.enabled || opts.print.FileTableFullProto.enabled) {
        if (opts.print.FileTableProto.enabled && opts.print.FileTableFullProto.enabled) {
            Exception::raise("file-table-proto and file-table-full-proto are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(gs, untypedUsages, opts.print.FileTableFullProto.enabled);
        if (opts.print.FileTableProto.outputPath.empty()) {
            files.SerializeToOstream(&cout);
        } else {
            string buf;
            files.SerializeToString(&buf);
            opts.print.FileTableProto.print(buf);
        }
    }
    if (opts.print.FileTableJson.enabled || opts.print.FileTableFullJson.enabled) {
        if (opts.print.FileTableJson.enabled && opts.print.FileTableFullJson.enabled) {
            Exception::raise("file-table-json and file-table-full-json are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(gs, untypedUsages, opts.print.FileTableFullJson.enabled);
        if (opts.print.FileTableJson.outputPath.empty()) {
            core::Proto::toJSON(files, cout);
        } else {
            stringstream buf;
            core::Proto::toJSON(files, buf);
            opts.print.FileTableJson.print(buf.str());
        }
    }
    if (opts.print.FileTableMessagePack.enabled || opts.print.FileTableFullMessagePack.enabled) {
        if (opts.print.FileTableMessagePack.enabled && opts.print.FileTableFullMessagePack.enabled) {
            Exception::raise("file-table-msgpack and file-table-full-msgpack are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(gs, untypedUsages, opts.print.FileTableFullMessagePack.enabled);
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
#endif
}

void printUntypedBlames(const core::GlobalState &gs, const UnorderedMap<long, long> &untypedBlames,
                        const options::Options &opts) {
#ifndef SORBET_REALMAIN_MIN

    if (!opts.print.UntypedBlame.enabled) {
        return;
    }

    rapidjson::StringBuffer result;
    rapidjson::Writer<rapidjson::StringBuffer> writer(result);

    writer.StartArray();

    for (auto &[symId, count] : untypedBlames) {
        auto sym = core::SymbolRef::fromRaw(symId);

        writer.StartObject();

        writer.String("path");
        if (sym.exists() && sym.loc(gs).exists()) {
            writer.String(string(sym.loc(gs).file().data(gs).path()));

        } else {
            writer.String("<none>");
        }

        writer.String("package");
        if (sym.exists()) {
            auto pkg = sym.enclosingClass(gs).data(gs)->package;
            if (!pkg.exists()) {
                writer.String("<none>");
            } else {
                writer.String(gs.packageDB().getPackageInfo(pkg).show(gs));
            }

        } else {
            writer.String("<none>");
        }

        writer.String("owner");
        auto owner = sym.owner(gs).show(gs);
        writer.String(owner);

        writer.String("name");
        writer.String(sym.name(gs).show(gs));

        writer.String("count");
        writer.Int64(count);

        writer.EndObject();
    }

    writer.EndArray();

    opts.print.UntypedBlame.print(result.GetString());
#endif
}

} // namespace sorbet::realmain::pipeline
