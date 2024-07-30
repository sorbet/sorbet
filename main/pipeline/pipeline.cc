#ifdef SORBET_REALMAIN_MIN
// minimal build to speedup compilation. Remove extra features
#else
// has to go first, as it violates poisons
#include "core/proto/proto.h"
// ^^ has to go first
#include "common/json2msgpack/json2msgpack.h"
#include "main/autogen/cache.h"
#include "main/autogen/constant_hash.h"
#include "packager/packager.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
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
#include "common/concurrency/ConcurrentQueue.h"
#include "common/crypto_hashing/crypto_hashing.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/timers/Timer.h"
#include "core/ErrorQueue.h"
#include "core/NameSubstitution.h"
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

extern "C" {
#include "prism.h"
}
#include "core/LocOffsets.h"
#include <iostream>

using namespace std;

namespace sorbet::realmain::pipeline {

class CFGCollectorAndTyper {
    const options::Options &opts;

public:
    CFGCollectorAndTyper(const options::Options &opts) : opts(opts){};

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &m = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (!infer::Inference::willRun(ctx, m.declLoc, m.symbol)) {
            return;
        }

        auto &print = opts.print;
        auto cfg = cfg::CFGBuilder::buildFor(ctx.withOwner(m.symbol), m);

        if (opts.stopAfterPhase != options::Phase::CFG) {
            cfg = infer::Inference::run(ctx.withOwner(cfg->symbol), move(cfg));
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

string fileKey(const core::File &file) {
    auto path = file.path();
    string key(path.begin(), path.end());
    key += "//";
    auto hashBytes = sorbet::crypto_hashing::hash64(file.source());
    key += absl::BytesToHexString(string_view{(char *)hashBytes.data(), size(hashBytes)});
    return key;
}

ast::ExpressionPtr fetchTreeFromCache(core::GlobalState &gs, core::FileRef fref, core::File &file,
                                      const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return nullptr;
    }

    if (fref.id() >= gs.filesUsed()) {
        prodCounterInc("types.input.files.kvstore.unindexed");
        return nullptr;
    }

    string fileHashKey = fileKey(file);
    auto maybeCached = kvstore->read(fileHashKey);
    if (maybeCached.data == nullptr) {
        prodCounterInc("types.input.files.kvstore.miss");
        return nullptr;
    }

    prodCounterInc("types.input.files.kvstore.hit");
    return core::serialize::Serializer::loadTree(gs, file, maybeCached.data);
}

unique_ptr<parser::Node> runParser(core::GlobalState &gs, core::FileRef file, const options::Printers &print,
                                   bool traceLexer, bool traceParser) {
    Timer timeit(gs.tracer(), "runParser", {{"file", string(file.data(gs).path())}});
    unique_ptr<parser::Node> nodes;
    {
        core::UnfreezeNameTable nameTableAccess(gs); // enters strings from source code as names
        auto indentationAware = false;               // Don't start in indentation-aware error recovery mode
        auto settings = parser::Parser::Settings{traceLexer, traceParser, indentationAware};
        nodes = parser::Parser::run(gs, file, settings);
    }

    if (print.ParseTree.enabled) {
        print.ParseTree.fmt("{}\n", nodes->toStringWithTabs(gs, 0));
    }
    if (print.ParseTreeJson.enabled) {
        print.ParseTreeJson.fmt("{}\n", nodes->toJSON(gs, 0));
    }
    if (print.ParseTreeJsonWithLocs.enabled) {
        print.ParseTreeJson.fmt("{}\n", nodes->toJSONWithLocs(gs, file, 0));
    }
    if (print.ParseTreeWhitequark.enabled) {
        print.ParseTreeWhitequark.fmt("{}\n", nodes->toWhitequark(gs, 0));
    }
    return nodes;
}

core::LocOffsets locOffset(pm_location_t *loc, pm_parser_t *parser) {
    uint32_t locStart = static_cast<uint32_t>(loc->start - parser->start);
    uint32_t locEnd = static_cast<uint32_t>(loc->end - parser->start);

    return core::LocOffsets{locStart, locEnd};
}

std::string_view prismConstantName(pm_constant_id_t name, pm_parser_t *parser) {
    pm_constant_pool_t *constantPool = &parser->constant_pool;
    pm_constant_t *constant = pm_constant_pool_id_to_constant(constantPool, name);

    return std::string_view(reinterpret_cast<const char *>(constant->start), constant->length);
}

unique_ptr<parser::Node> convertPrismToSorbet(pm_node_t *node, pm_parser_t *parser, core::GlobalState &gs) {
    switch (PM_NODE_TYPE(node)) {
        case PM_DEF_NODE: {
            auto defNode = reinterpret_cast<pm_def_node *>(node);
            pm_location_t *loc = &defNode->base.location;
            pm_location_t *declLoc = &defNode->def_keyword_loc;

            std::string_view name = prismConstantName(defNode->name, parser);

            unique_ptr<parser::Node> params;
            unique_ptr<parser::Node> body;

            if (defNode->body != nullptr) {
                body = convertPrismToSorbet(defNode->body, parser, gs);
            }

            if (defNode->parameters != nullptr) {
                params = convertPrismToSorbet(reinterpret_cast<pm_node *>(defNode->parameters), parser, gs);
            }

            return make_unique<parser::DefMethod>(locOffset(loc, parser), locOffset(declLoc, parser),
                                                  gs.enterNameUTF8(name), std::move(params), std::move(body));
        }
        case PM_ELSE_NODE: {
            auto elseNode = reinterpret_cast<pm_else_node *>(node);

            if (elseNode->statements == nullptr)
                return nullptr;

            return convertPrismToSorbet(reinterpret_cast<pm_node *>(elseNode->statements), parser, gs);
        }
        case PM_FALSE_NODE: {
            auto falseNode = reinterpret_cast<pm_false_node *>(node);
            pm_location_t *loc = &falseNode->base.location;

            return make_unique<parser::False>(locOffset(loc, parser));
        }
        case PM_FLOAT_NODE: {
            auto floatNode = reinterpret_cast<pm_float_node *>(node);
            pm_location_t *loc = &floatNode->base.location;

            return make_unique<parser::Float>(locOffset(loc, parser), std::to_string(floatNode->value));
        }
        case PM_IF_NODE: {
            auto ifNode = reinterpret_cast<pm_if_node *>(node);
            auto *loc = &ifNode->base.location;

            auto predicate = convertPrismToSorbet(ifNode->predicate, parser, gs);

            std::unique_ptr<parser::Node> ifTrue;
            std::unique_ptr<parser::Node> ifFalse;

            if (ifNode->statements != nullptr) {
                ifTrue = convertPrismToSorbet(reinterpret_cast<pm_node *>(ifNode->statements), parser, gs);
            }

            if (ifNode->consequent != nullptr) {
                ifFalse = convertPrismToSorbet(ifNode->consequent, parser, gs);
            }

            return make_unique<parser::If>(locOffset(loc, parser), std::move(predicate), std::move(ifTrue),
                                           std::move(ifFalse));
        }
        case PM_INTEGER_NODE: {
            auto intNode = reinterpret_cast<pm_integer_node *>(node);
            pm_location_t *loc = &intNode->base.location;

            // Will only work for positive, 32-bit integers
            return make_unique<parser::Integer>(locOffset(loc, parser), std::to_string(intNode->value.value));
        }
        case PM_PARAMETERS_NODE: {
            auto paramsNode = reinterpret_cast<pm_parameters_node *>(node);
            pm_location_t *loc = &paramsNode->base.location;

            pm_node_list_t *requiredsList = &paramsNode->requireds;
            auto requireds = absl::MakeSpan(requiredsList->nodes, requiredsList->size);

            parser::NodeVec params;
            params.reserve(requireds.size());

            for (auto &param : requireds) {
                unique_ptr<parser::Node> sorbetParam = convertPrismToSorbet(param, parser, gs);
                params.emplace_back(std::move(sorbetParam));
            }

            return make_unique<parser::Args>(locOffset(loc, parser), std::move(params));
        }
        case PM_PROGRAM_NODE: {
            pm_program_node *programNode = reinterpret_cast<pm_program_node *>(node);

            return convertPrismToSorbet(reinterpret_cast<pm_node *>(programNode->statements), parser, gs);
        }
        case PM_RATIONAL_NODE: {
            auto *rationalNode = reinterpret_cast<pm_rational_node *>(node);
            pm_location_t *loc = &rationalNode->base.location;

            const uint8_t *start = rationalNode->numeric->location.start;
            const uint8_t *end = rationalNode->numeric->location.end;

            std::string value = std::string(reinterpret_cast<const char *>(start), end - start);

            return make_unique<parser::Rational>(locOffset(loc, parser), value);
        }
        case PM_REQUIRED_PARAMETER_NODE: {
            auto requiredParamNode = reinterpret_cast<pm_required_parameter_node *>(node);
            pm_location_t *loc = &requiredParamNode->base.location;

            std::string_view name = prismConstantName(requiredParamNode->name, parser);

            return make_unique<parser::Arg>(locOffset(loc, parser), gs.enterNameUTF8(name));
        }
        case PM_STATEMENTS_NODE: {
            pm_statements_node *stmts_node = reinterpret_cast<pm_statements_node *>(node);

            auto stmts = absl::MakeSpan(stmts_node->body.nodes, stmts_node->body.size);

            // For a single statement, do not create a Begin node and just return the statement
            if (stmts.size() == 1) {
                return convertPrismToSorbet((pm_node *)stmts.front(), parser, gs);
            }

            // For multiple statements, convert each statement and add them to the body of a Begin node
            parser::NodeVec sorbetStmts;
            sorbetStmts.reserve(stmts.size());

            for (auto &node : stmts) {
                unique_ptr<parser::Node> convertedStmt = convertPrismToSorbet(node, parser, gs);
                sorbetStmts.emplace_back(std::move(convertedStmt));
            }

            auto *loc = &stmts_node->base.location;

            return make_unique<parser::Begin>(locOffset(loc, parser), std::move(sorbetStmts));
        }
        case PM_STRING_NODE: {
            auto strNode = reinterpret_cast<pm_string_node *>(node);
            pm_location_t *loc = &strNode->base.location;

            auto unescaped = &strNode->unescaped;
            auto source =
                std::string(reinterpret_cast<const char *>(pm_string_source(unescaped)), pm_string_length(unescaped));

            // TODO: handle different string encodings
            return make_unique<parser::String>(locOffset(loc, parser), gs.enterNameUTF8(source));
        }
        case PM_TRUE_NODE: {
            auto trueNode = reinterpret_cast<pm_true_node *>(node);
            pm_location_t *loc = &trueNode->base.location;

            return make_unique<parser::True>(locOffset(loc, parser));
        }

        case PM_ALIAS_GLOBAL_VARIABLE_NODE:
        case PM_ALIAS_METHOD_NODE:
        case PM_ALTERNATION_PATTERN_NODE:
        case PM_AND_NODE:
        case PM_ARGUMENTS_NODE:
        case PM_ARRAY_NODE:
        case PM_ARRAY_PATTERN_NODE:
        case PM_ASSOC_NODE:
        case PM_ASSOC_SPLAT_NODE:
        case PM_BACK_REFERENCE_READ_NODE:
        case PM_BEGIN_NODE:
        case PM_BLOCK_ARGUMENT_NODE:
        case PM_BLOCK_LOCAL_VARIABLE_NODE:
        case PM_BLOCK_NODE:
        case PM_BLOCK_PARAMETER_NODE:
        case PM_BLOCK_PARAMETERS_NODE:
        case PM_BREAK_NODE:
        case PM_CALL_AND_WRITE_NODE:
        case PM_CALL_NODE:
        case PM_CALL_OPERATOR_WRITE_NODE:
        case PM_CALL_OR_WRITE_NODE:
        case PM_CALL_TARGET_NODE:
        case PM_CAPTURE_PATTERN_NODE:
        case PM_CASE_MATCH_NODE:
        case PM_CASE_NODE:
        case PM_CLASS_NODE:
        case PM_CLASS_VARIABLE_AND_WRITE_NODE:
        case PM_CLASS_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_CLASS_VARIABLE_OR_WRITE_NODE:
        case PM_CLASS_VARIABLE_READ_NODE:
        case PM_CLASS_VARIABLE_TARGET_NODE:
        case PM_CLASS_VARIABLE_WRITE_NODE:
        case PM_CONSTANT_AND_WRITE_NODE:
        case PM_CONSTANT_OPERATOR_WRITE_NODE:
        case PM_CONSTANT_OR_WRITE_NODE:
        case PM_CONSTANT_PATH_AND_WRITE_NODE:
        case PM_CONSTANT_PATH_NODE:
        case PM_CONSTANT_PATH_OPERATOR_WRITE_NODE:
        case PM_CONSTANT_PATH_OR_WRITE_NODE:
        case PM_CONSTANT_PATH_TARGET_NODE:
        case PM_CONSTANT_PATH_WRITE_NODE:
        case PM_CONSTANT_READ_NODE:
        case PM_CONSTANT_TARGET_NODE:
        case PM_CONSTANT_WRITE_NODE:
        case PM_DEFINED_NODE:
        case PM_EMBEDDED_STATEMENTS_NODE:
        case PM_EMBEDDED_VARIABLE_NODE:
        case PM_ENSURE_NODE:
        case PM_FIND_PATTERN_NODE:
        case PM_FLIP_FLOP_NODE:
        case PM_FOR_NODE:
        case PM_FORWARDING_ARGUMENTS_NODE:
        case PM_FORWARDING_PARAMETER_NODE:
        case PM_FORWARDING_SUPER_NODE:
        case PM_GLOBAL_VARIABLE_AND_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_OR_WRITE_NODE:
        case PM_GLOBAL_VARIABLE_READ_NODE:
        case PM_GLOBAL_VARIABLE_TARGET_NODE:
        case PM_GLOBAL_VARIABLE_WRITE_NODE:
        case PM_HASH_NODE:
        case PM_HASH_PATTERN_NODE:
        case PM_IMAGINARY_NODE:
        case PM_IMPLICIT_NODE:
        case PM_IMPLICIT_REST_NODE:
        case PM_IN_NODE:
        case PM_INDEX_AND_WRITE_NODE:
        case PM_INDEX_OPERATOR_WRITE_NODE:
        case PM_INDEX_OR_WRITE_NODE:
        case PM_INDEX_TARGET_NODE:
        case PM_INSTANCE_VARIABLE_AND_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_OR_WRITE_NODE:
        case PM_INSTANCE_VARIABLE_READ_NODE:
        case PM_INSTANCE_VARIABLE_TARGET_NODE:
        case PM_INSTANCE_VARIABLE_WRITE_NODE:
        case PM_INTERPOLATED_MATCH_LAST_LINE_NODE:
        case PM_INTERPOLATED_REGULAR_EXPRESSION_NODE:
        case PM_INTERPOLATED_STRING_NODE:
        case PM_INTERPOLATED_SYMBOL_NODE:
        case PM_INTERPOLATED_X_STRING_NODE:
        case PM_IT_PARAMETERS_NODE:
        case PM_KEYWORD_HASH_NODE:
        case PM_KEYWORD_REST_PARAMETER_NODE:
        case PM_LAMBDA_NODE:
        case PM_LOCAL_VARIABLE_AND_WRITE_NODE:
        case PM_LOCAL_VARIABLE_OPERATOR_WRITE_NODE:
        case PM_LOCAL_VARIABLE_OR_WRITE_NODE:
        case PM_LOCAL_VARIABLE_READ_NODE:
        case PM_LOCAL_VARIABLE_TARGET_NODE:
        case PM_LOCAL_VARIABLE_WRITE_NODE:
        case PM_MATCH_LAST_LINE_NODE:
        case PM_MATCH_PREDICATE_NODE:
        case PM_MATCH_REQUIRED_NODE:
        case PM_MATCH_WRITE_NODE:
        case PM_MISSING_NODE:
        case PM_MODULE_NODE:
        case PM_MULTI_TARGET_NODE:
        case PM_MULTI_WRITE_NODE:
        case PM_NEXT_NODE:
        case PM_NIL_NODE:
        case PM_NO_KEYWORDS_PARAMETER_NODE:
        case PM_NUMBERED_PARAMETERS_NODE:
        case PM_NUMBERED_REFERENCE_READ_NODE:
        case PM_OPTIONAL_KEYWORD_PARAMETER_NODE:
        case PM_OPTIONAL_PARAMETER_NODE:
        case PM_OR_NODE:
        case PM_PARENTHESES_NODE:
        case PM_PINNED_EXPRESSION_NODE:
        case PM_PINNED_VARIABLE_NODE:
        case PM_POST_EXECUTION_NODE:
        case PM_PRE_EXECUTION_NODE:
        case PM_RANGE_NODE:
        case PM_REDO_NODE:
        case PM_REGULAR_EXPRESSION_NODE:
        case PM_REQUIRED_KEYWORD_PARAMETER_NODE:
        case PM_RESCUE_MODIFIER_NODE:
        case PM_RESCUE_NODE:
        case PM_REST_PARAMETER_NODE:
        case PM_RETRY_NODE:
        case PM_RETURN_NODE:
        case PM_SELF_NODE:
        case PM_SHAREABLE_CONSTANT_NODE:
        case PM_SINGLETON_CLASS_NODE:
        case PM_SOURCE_ENCODING_NODE:
        case PM_SOURCE_FILE_NODE:
        case PM_SOURCE_LINE_NODE:
        case PM_SPLAT_NODE:
        case PM_SUPER_NODE:
        case PM_SYMBOL_NODE:
        case PM_UNDEF_NODE:
        case PM_UNLESS_NODE:
        case PM_UNTIL_NODE:
        case PM_WHEN_NODE:
        case PM_WHILE_NODE:
        case PM_X_STRING_NODE:
        case PM_YIELD_NODE:
        case PM_SCOPE_NODE:
            std::unique_ptr<parser::Node> ast;
            return ast;
    }
}

unique_ptr<parser::Node> runPrismParser(core::GlobalState &gs, core::FileRef file, bool stopAfterParser) {
    auto source = file.data(gs).source();

    core::UnfreezeNameTable nameTableAccess(gs);

    pm_parser_t parser;
    pm_parser_init(&parser, reinterpret_cast<const uint8_t *>(source.data()), source.size(), NULL);

    pm_node_t *root = pm_parse(&parser);

    if (stopAfterParser) {
        return std::unique_ptr<parser::Node>();
    }

    std::unique_ptr<parser::Node> ast = convertPrismToSorbet(root, &parser, gs);

    pm_node_destroy(&parser, root);
    pm_parser_free(&parser);

    return ast;
}

ast::ExpressionPtr runDesugar(core::GlobalState &gs, core::FileRef file, unique_ptr<parser::Node> parseTree,
                              const options::Printers &print, bool preserveConcreteSyntax = false) {
    Timer timeit(gs.tracer(), "runDesugar", {{"file", string(file.data(gs).path())}});
    ast::ExpressionPtr ast;
    core::MutableContext ctx(gs, core::Symbols::root(), file);
    {
        core::UnfreezeNameTable nameTableAccess(gs); // creates temporaries during desugaring
        ast = ast::desugar::node2Tree(ctx, move(parseTree), preserveConcreteSyntax);
    }
    if (print.DesugarTree.enabled) {
        print.DesugarTree.fmt("{}\n", ast.toStringWithTabs(gs, 0));
    }
    if (print.DesugarTreeRaw.enabled) {
        print.DesugarTreeRaw.fmt("{}\n", ast.showRaw(gs));
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
    Timer timeit(gs.tracer(), "runLocalVars", {{"file", string(tree.file.data(gs).path())}});
    core::MutableContext ctx(gs, core::Symbols::root(), tree.file);
    return sorbet::local_vars::LocalVars::run(ctx, move(tree));
}

ast::ParsedFile emptyParsedFile(core::FileRef file) {
    return {ast::MK::EmptyTree(), file};
}

ast::ExpressionPtr desugarOne(const options::Options &opts, core::GlobalState &gs, core::FileRef file,
                              bool preserveConcreteSyntax) {
    auto &print = opts.print;

    Timer timeit(gs.tracer(), "desugarOne", {{"file", string(file.data(gs).path())}});
    try {
        if (file.data(gs).strictLevel == core::StrictLevel::Ignore) {
            return ast::MK::EmptyTree();
        }
        auto parseTree = runParser(gs, file, print, opts.traceLexer, opts.traceParser);
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
    auto &parser = opts.parser;

    ast::ParsedFile rewritten{nullptr, file};

    Timer timeit(lgs.tracer(), "indexOne", {{"file", string(file.data(lgs).path())}});
    try {
        if (!tree) {
            // tree isn't cached. Need to start from parser
            if (file.data(lgs).strictLevel == core::StrictLevel::Ignore) {
                return emptyParsedFile(file);
            }

            unique_ptr<parser::Node> parseTree;

            bool stopAfterParser = opts.stopAfterPhase == options::Phase::PARSER;

            if (parser == options::Parser::SORBET) {
                parseTree = runParser(lgs, file, print, opts.traceLexer, opts.traceParser);
            } else if (parser == options::Parser::PRISM) {
                parseTree = runPrismParser(lgs, file, stopAfterParser);
            } // Any other option would have been handled in the options parser

            if (stopAfterParser) {
                return emptyParsedFile(file);
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

vector<ast::ParsedFile>
incrementalResolve(core::GlobalState &gs, vector<ast::ParsedFile> what,
                   optional<UnorderedMap<core::FileRef, core::FoundDefHashes>> &&foundHashesForFiles,
                   const options::Options &opts, WorkerPool &workers) {
    try {
#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
            Timer timeit(gs.tracer(), "incremental_packager");
            // For simplicity, we still call Packager::runIncremental here, even though
            // pipeline::nameAndResolve no longer calls Packager::run.
            //
            // TODO(jez) We may want to revisit this. At the moment, the only thing that
            // runIncremental does is validate that files have the right package prefix. We could
            // split `pipeline::package` into something like "populate the package DB" and "verify
            // the package prefixes" with the later living in `pipeline::nameAndResolve` once again
            // (thus restoring the symmetry).
            // TODO(jez) Parallelize this
            what = packager::Packager::runIncremental(gs, move(what));
        }
#endif
        auto runIncrementalNamer = foundHashesForFiles.has_value() && !foundHashesForFiles->empty();
        {
            Timer timeit(gs.tracer(), "incremental_naming");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            auto canceled = runIncrementalNamer
                                ? sorbet::namer::Namer::runIncremental(gs, absl::Span<ast::ParsedFile>(what),
                                                                       std::move(foundHashesForFiles.value()), workers)
                                : sorbet::namer::Namer::run(gs, absl::Span<ast::ParsedFile>(what), workers, nullptr);

            // Cancellation cannot occur during incremental namer.
            ENFORCE(!canceled);

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::NAMER) {
                return what;
            }
        }

        {
            Timer timeit(gs.tracer(), "incremental_resolve");
            gs.tracer().trace("Resolving (incremental pass)...");
            core::UnfreezeSymbolTable symbolTable(gs);
            core::UnfreezeNameTable nameTable(gs);

            auto result = sorbet::resolver::Resolver::runIncremental(gs, move(what), runIncrementalNamer, workers);
            // incrementalResolve is not cancelable.
            ENFORCE(result.hasResult());
            what = move(result.result());

            // Required for autogen tests, which need to control which phase to stop after.
            if (opts.stopAfterPhase == options::Phase::RESOLVER) {
                return what;
            }
        }

#ifndef SORBET_REALMAIN_MIN
        if (opts.stripePackages) {
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

vector<core::FileRef> reserveFiles(unique_ptr<core::GlobalState> &gs, const vector<string> &files) {
    Timer timeit(gs->tracer(), "reserveFiles");
    vector<core::FileRef> ret;
    ret.reserve(files.size());
    core::UnfreezeFileTable unfreezeFiles(*gs);
    for (auto &f : files) {
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
    ast::ExpressionPtr ast;
    if (file.dataAllowingUnsafe(gs).sourceType != core::File::Type::NotYetRead) {
        return ast;
    }
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

    {
        core::UnfreezeFileTable unfreezeFiles(gs);
        auto fileObj = make_shared<core::File>(move(fileName), move(src), core::File::Type::Normal);
        // Returns nullptr if tree is not in cache.
        ast = fetchTreeFromCache(gs, file, *fileObj, kvstore);

        auto entered = gs.enterNewFileAt(move(fileObj), file);
        ENFORCE(entered == file);
    }
    if (enable_counters) {
        counterAdd("types.input.lines", file.data(gs).lineCount());
    }

    auto &fileData = file.data(gs);
    if (!fileFound) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::FileNotFound)) {
            e.setHeader("File Not Found");
        }
    }

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
};

struct IndexThreadResultPack {
    CounterState counters;
    IndexResult res;
};

struct IndexSubstitutionJob {
    // Not necessary for substitution, but passing this through to the worker means it's freed in that thread, instead
    // of serially in the main thread.
    unique_ptr<core::GlobalState> threadGs;

    std::optional<core::NameSubstitution> subst;
    vector<ast::ParsedFile> trees;

    IndexSubstitutionJob() {}

    IndexSubstitutionJob(core::GlobalState &to, IndexResult res)
        : threadGs{std::move(res.gs)}, subst{}, trees{std::move(res.trees)} {
        to.mergeFileTable(*this->threadGs);
        if (absl::c_any_of(this->trees, [this](auto &parsed) { return !parsed.file.data(*this->threadGs).cached(); })) {
            this->subst.emplace(*this->threadGs, to);
        }
    }

    IndexSubstitutionJob(IndexSubstitutionJob &&other) = default;
    IndexSubstitutionJob &operator=(IndexSubstitutionJob &&other) = default;
};

vector<ast::ParsedFile> mergeIndexResults(core::GlobalState &cgs, const options::Options &opts,
                                          shared_ptr<BlockingBoundedQueue<IndexThreadResultPack>> input,
                                          WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore) {
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
                auto numTrees = threadResult.res.trees.size();
                batchq->push(IndexSubstitutionJob{cgs, std::move(threadResult.res)}, numTrees);
                totalNumTrees += numTrees;
            }
        }
    }

    {
        Timer timeit(cgs.tracer(), "substituteTrees");
        auto resultq = make_shared<BlockingBoundedQueue<vector<ast::ParsedFile>>>(batchq->bound);

        workers.multiplexJob("substituteTrees", [&cgs, batchq, resultq]() {
            Timer timeit(cgs.tracer(), "substituteTreesWorker");
            IndexSubstitutionJob job;
            for (auto result = batchq->try_pop(job); !result.done(); result = batchq->try_pop(job)) {
                if (result.gotItem()) {
                    if (job.subst.has_value()) {
                        for (auto &tree : job.trees) {
                            auto file = tree.file;
                            if (!file.data(cgs).cached()) {
                                core::MutableContext ctx(cgs, core::Symbols::root(), file);
                                tree = ast::Substitute::run(ctx, *job.subst, move(tree));
                            }
                        }
                    }
                    auto numSubstitutedTrees = job.trees.size();
                    resultq->push(std::move(job.trees), numSubstitutedTrees);
                }
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

    return ret;
}

vector<ast::ParsedFile> indexSuppliedFiles(core::GlobalState &baseGs, absl::Span<core::FileRef> files,
                                           const options::Options &opts, WorkerPool &workers,
                                           const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    auto resultq = make_shared<BlockingBoundedQueue<IndexThreadResultPack>>(files.size());
    auto fileq = make_shared<ConcurrentBoundedQueue<core::FileRef>>(files.size());
    for (auto &file : files) {
        fileq->push(move(file), 1);
    }

    std::shared_ptr<const core::GlobalState> emptyGs = baseGs.copyForIndex();

    workers.multiplexJob("indexSuppliedFiles", [emptyGs, &opts, fileq, resultq, &kvstore]() {
        Timer timeit(emptyGs->tracer(), "indexSuppliedFilesWorker");

        // clone the empty global state to avoid manually re-entering everything, and copy the base filetable so that
        // file sources are available.
        unique_ptr<core::GlobalState> localGs = emptyGs->deepCopy();

        IndexThreadResultPack threadResult;

        {
            core::FileRef job;
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    core::FileRef file = job;
                    auto cachedTree = readFileWithStrictnessOverrides(*localGs, file, opts, kvstore);
                    auto parsedFile = indexOne(opts, *localGs, file, move(cachedTree));
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

    return mergeIndexResults(baseGs, opts, resultq, workers, kvstore);
}

vector<ast::ParsedFile> index(core::GlobalState &gs, absl::Span<core::FileRef> files, const options::Options &opts,
                              WorkerPool &workers, const unique_ptr<const OwnedKeyValueStore> &kvstore) {
    Timer timeit(gs.tracer(), "index");
    vector<ast::ParsedFile> ret;
    vector<ast::ParsedFile> empty;

    if (opts.stopAfterPhase == options::Phase::INIT) {
        return empty;
    }

    gs.sanityCheck();

    if (files.size() < 3) {
        // Run singlethreaded if only using 2 files
        for (auto file : files) {
            auto tree = readFileWithStrictnessOverrides(gs, file, opts, kvstore);
            auto parsedFile = indexOne(opts, gs, file, move(tree));
            ret.emplace_back(move(parsedFile));
        }
        ENFORCE(files.size() == ret.size());
    } else {
        ret = indexSuppliedFiles(gs, files, opts, workers, kvstore);
    }

    // TODO(jez) Do we want this fast_sort here? Is it redundant?
    fast_sort(ret, [](ast::ParsedFile const &a, ast::ParsedFile const &b) { return a.file < b.file; });
    return ret;
}

namespace {
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

    resolved = definition_validator::runOne(ctx, std::move(resolved));

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

size_t partitionPackageFiles(const core::GlobalState &gs, absl::Span<core::FileRef> inputFiles) {
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

void setPackagerOptions(core::GlobalState &gs, const options::Options &opts) {
#ifndef SORBET_REALMAIN_MIN
    if (!opts.stripePackages) {
        return;
    }

    {
        core::UnfreezeNameTable unfreezeToEnterPackagerOptionsGS(gs);
        core::packages::UnfreezePackages unfreezeToEnterPackagerOptionsPackageDB = gs.unfreezePackages();
        gs.setPackagerOptions(opts.extraPackageFilesDirectoryUnderscorePrefixes,
                              opts.extraPackageFilesDirectorySlashPrefixes, opts.packageSkipRBIExportEnforcementDirs,
                              opts.allowRelaxedPackagerChecksFor, opts.stripePackagesHint);
    }
#endif
}

// packager intentionally runs outside of rewriter so that its output does not get cached.
// TODO(jez) How much of this still needs to be outside of rewriter?
void package(core::GlobalState &gs, absl::Span<ast::ParsedFile> what, const options::Options &opts,
             WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    if (!opts.stripePackages) {
        return;
    }

    try {
        packager::Packager::run(gs, workers, what);
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

[[nodiscard]] bool name(core::GlobalState &gs, absl::Span<ast::ParsedFile> what, const options::Options &opts,
                        WorkerPool &workers, core::FoundDefHashes *foundHashes) {
    Timer timeit(gs.tracer(), "name");
    core::UnfreezeNameTable nameTableAccess(gs);     // creates singletons and class names
    core::UnfreezeSymbolTable symbolTableAccess(gs); // enters symbols
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
            auto *lit = ast::cast_tree<ast::ConstantLit>(classDef.ancestors.front());
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
        auto detailStart = core::Loc::offset2Pos(file.data(ctx), loc.beginPos());
        auto detailEnd = core::Loc::offset2Pos(file.data(ctx), loc.endPos());
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
    ast::TreeWalk::apply(core::Context(gs, core::Symbols::root(), what.file), enforcer, what.tree);
    return what;
}

ast::ParsedFilesOrCancelled nameAndResolve(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                           const options::Options &opts, WorkerPool &workers,
                                           core::FoundDefHashes *foundHashes) {
    auto canceled = name(*gs, absl::Span<ast::ParsedFile>(what), opts, workers, foundHashes);
    if (canceled) {
        return ast::ParsedFilesOrCancelled::cancel(move(what), workers);
    }

    return resolve(gs, move(what), opts, workers);
}

ast::ParsedFilesOrCancelled resolve(unique_ptr<core::GlobalState> &gs, vector<ast::ParsedFile> what,
                                    const options::Options &opts, WorkerPool &workers) {
    try {
        if (opts.stopAfterPhase != options::Phase::NAMER) {
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

#ifndef SORBET_REALMAIN_MIN
            if (opts.stripePackages) {
                Timer timeit(gs->tracer(), "visibility_checker");
                what = packager::VisibilityChecker::run(*gs, workers, std::move(what));
            }
#endif

            if (opts.stressIncrementalResolver) {
                auto symbolsBefore = gs->symbolsUsedTotal();
                for (auto &f : what) {
                    // Shift contents of file past current file's EOF, re-run incrementalResolve, assert that no
                    // locations appear before file's old EOF.
                    const int prohibitedLines = f.file.data(*gs).source().size();
                    auto newSource = fmt::format("{}\n{}", string(prohibitedLines, '\n'), f.file.data(*gs).source());
                    auto newFile = make_shared<core::File>(string(f.file.data(*gs).path()), move(newSource),
                                                           f.file.data(*gs).sourceType);
                    gs->replaceFile(f.file, move(newFile));
                    f.file.data(*gs).strictLevel = decideStrictLevel(*gs, f.file, opts);
                    auto reIndexed = indexOne(opts, *gs, f.file);
                    vector<ast::ParsedFile> toBeReResolved;
                    toBeReResolved.emplace_back(move(reIndexed));
                    // We don't compute file hashes when running for incrementalResolve.
                    auto foundHashesForFiles = nullopt;
                    auto reresolved =
                        pipeline::incrementalResolve(*gs, move(toBeReResolved), foundHashesForFiles, opts, workers);
                    ENFORCE(reresolved.size() == 1);
                    f = checkNoDefinitionsInsideProhibitedLines(*gs, move(reresolved[0]), 0, prohibitedLines);
                }
                ENFORCE(symbolsBefore == gs->symbolsUsedTotal(),
                        "Stressing the incremental resolver should not add any new symbols");
            }
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
                opts.print.ResolveTree.fmt("{}\n", resolved.tree.toString(*gs));
            }
            if (opts.print.ResolveTreeRaw.enabled) {
                opts.print.ResolveTreeRaw.fmt("{}\n", resolved.tree.showRaw(*gs));
            }
        }
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

    if (opts.print.MissingConstants.enabled) {
        what = printMissingConstants(*gs, opts, move(what));
    }

    return ast::ParsedFilesOrCancelled(move(what));
}

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

        shared_ptr<ConcurrentBoundedQueue<ast::ParsedFile>> fileq;
        shared_ptr<BlockingBoundedQueue<vector<core::FileRef>>> outputq;

        {
            fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
            outputq = make_shared<BlockingBoundedQueue<vector<core::FileRef>>>(what.size());
        }

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
                vector<core::FileRef> processedFiles;
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
                                processedFiles.emplace_back(file);
                                outputq->push(move(processedFiles), processedByThread);
                                processedByThread = 0;
                            }
                        }
                    }
                }
                if (processedByThread > 0) {
                    outputq->push(move(processedFiles), processedByThread);
                }
            });

            vector<core::FileRef> files;
            {
                for (auto result = outputq->wait_pop_timed(files, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
                     !result.done();
                     result = outputq->wait_pop_timed(files, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
                    if (result.gotItem()) {
                        for (auto &file : files) {
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

void printFileTable(unique_ptr<core::GlobalState> &gs, const options::Options &opts,
                    const UnorderedMap<long, long> &untypedUsages) {
#ifndef SORBET_REALMAIN_MIN
    if (opts.print.FileTableProto.enabled || opts.print.FileTableFullProto.enabled) {
        if (opts.print.FileTableProto.enabled && opts.print.FileTableFullProto.enabled) {
            Exception::raise("file-table-proto and file-table-full-proto are mutually exclusive print options");
        }
        auto files = core::Proto::filesToProto(*gs, untypedUsages, opts.print.FileTableFullProto.enabled);
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
        auto files = core::Proto::filesToProto(*gs, untypedUsages, opts.print.FileTableFullJson.enabled);
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
        auto files = core::Proto::filesToProto(*gs, untypedUsages, opts.print.FileTableFullMessagePack.enabled);
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

bool cacheTreesAndFiles(const core::GlobalState &gs, WorkerPool &workers, absl::Span<const ast::ParsedFile> parsedFiles,
                        const unique_ptr<OwnedKeyValueStore> &kvstore) {
    if (kvstore == nullptr) {
        return false;
    }

    Timer timeit(gs.tracer(), "pipeline::cacheTreesAndFiles");

    // Compress files in parallel.
    auto fileq = make_shared<ConcurrentBoundedQueue<const ast::ParsedFile *>>(parsedFiles.size());
    for (auto &parsedFile : parsedFiles) {
        fileq->push(&parsedFile, 1);
    }

    auto resultq = make_shared<BlockingBoundedQueue<vector<pair<string, vector<uint8_t>>>>>(parsedFiles.size());
    workers.multiplexJob("compressTreesAndFiles", [fileq, resultq, &gs]() {
        vector<pair<string, vector<uint8_t>>> threadResult;
        int processedByThread = 0;
        const ast::ParsedFile *job = nullptr;
        unique_ptr<Timer> timeit;
        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;
                    if (timeit == nullptr) {
                        timeit = make_unique<Timer>(gs.tracer(), "cacheTreesAndFilesWorker");
                    }

                    if (!job->file.exists()) {
                        continue;
                    }

                    auto &file = job->file.data(gs);
                    if (!file.cached() && !file.hasParseErrors()) {
                        threadResult.emplace_back(fileKey(file), core::serialize::Serializer::storeTree(file, *job));
                        // Stream out compressed files so that writes happen in parallel with processing.
                        if (processedByThread > 100) {
                            resultq->push(move(threadResult), processedByThread);
                            processedByThread = 0;
                        }
                    }
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    size_t written = 0;
    {
        vector<pair<string, vector<uint8_t>>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer());
             !result.done();
             result = resultq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    kvstore->write(move(a.first), move(a.second));
                    written++;
                }
            }
        }
    }
    prodCounterAdd("types.input.files.kvstore.write", written);
    return written != 0;
}

vector<ast::ParsedFile> autogenWriteCacheFile(const core::GlobalState &gs, const string &cachePath,
                                              vector<ast::ParsedFile> what, WorkerPool &workers) {
#ifndef SORBET_REALMAIN_MIN
    Timer timeit(gs.tracer(), "autogenWriteCacheFile");

    auto fileq = make_shared<ConcurrentBoundedQueue<ast::ParsedFile>>(what.size());
    auto resultq = make_shared<BlockingBoundedQueue<autogen::HashedParsedFile>>(what.size());
    for (auto &pf : what) {
        fileq->push(move(pf), 1);
    }

    workers.multiplexJob("computeConstantCache", [&]() {
        ast::ParsedFile job;
        for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
            resultq->push(autogen::constantHashTree(gs, move(job)), 1);
        }
    });

    autogen::AutogenCache cache;
    vector<ast::ParsedFile> results;

    {
        autogen::HashedParsedFile output;
        for (auto result = resultq->wait_pop_timed(output, WorkerPool::BLOCK_INTERVAL(), gs.tracer()); !result.done();
             result = resultq->wait_pop_timed(output, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
            if (!result.gotItem()) {
                continue;
            }
            cache.add(string(output.pf.file.data(gs).path()), output.constantHash);
            results.emplace_back(move(output.pf));
        }
    }

    FileOps::write(cachePath, cache.pack());

    return results;
#else
    return what;
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
            writer.String(std::string(sym.loc(gs).file().data(gs).path()));

        } else {
            writer.String("<none>");
        }

        writer.String("package");
        if (sym.exists() && sym.loc(gs).exists()) {
            const auto file = sym.loc(gs).file();
            const auto &pkg = gs.packageDB().getPackageForFile(gs, file);
            if (!pkg.exists()) {
                writer.String("<none>");
            } else {
                writer.String(pkg.show(gs));
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
