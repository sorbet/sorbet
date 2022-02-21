#include "main/lsp/requests/code_action.h"
#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "sig_finder/sig_finder.h"
#include <optional>
#include <vector>

using namespace std;

namespace sorbet::realmain::lsp {
CodeActionTask::CodeActionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CodeActionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCodeAction), params(move(params)) {}

namespace {
vector<unique_ptr<TextDocumentEdit>> getEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                              const vector<core::AutocorrectSuggestion::Edit> &edits) {
    UnorderedMap<string, vector<unique_ptr<TextEdit>>> editsByFile;
    for (auto &edit : edits) {
        auto range = Range::fromLoc(gs, edit.loc);
        if (range != nullptr) {
            editsByFile[config.fileRef2Uri(gs, edit.loc.file())].emplace_back(
                make_unique<TextEdit>(std::move(range), edit.replacement));
        }
    }

    vector<unique_ptr<TextDocumentEdit>> documentEdits;
    for (auto &it : editsByFile) {
        // TODO: Document version
        documentEdits.emplace_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(it.first, JSONNullObject()), move(it.second)));
    }
    return documentEdits;
}

std::optional<const ast::MethodDef*> findMethodTree(const ast::ExpressionPtr &tree, const core::SymbolRef &method) {
    if (auto seq = ast::cast_tree<ast::InsSeq>(tree)) {
        for (auto &subtree : seq->stats) {
            auto maybeMethod = findMethodTree(subtree, method);
            if (maybeMethod.has_value()) {
                return maybeMethod.value();
            }
        }
    } else if (auto klass = ast::cast_tree<ast::ClassDef>(tree)) {
        for (auto &subtree : klass->rhs) {
            auto maybeMethod = findMethodTree(subtree, method);
            if (maybeMethod.has_value()) {
                return maybeMethod.value();
            }
        }
    } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(tree)) {
        if (methodDef->symbol == method.asMethodRef()) {
            return methodDef;
        }
    }
    return std::nullopt;
}

std::optional<std::pair<core::LocOffsets, core::LocOffsets>> methodLocs(const core::GlobalState &gs, const ast::ExpressionPtr &rootTree, const core::SymbolRef &method, const core::FileRef &fref) {
    auto maybeTree = findMethodTree(rootTree, method.asMethodRef());
    if (!maybeTree.has_value()) {
        return std::nullopt;
    }
    auto methodLoc = maybeTree.value()->loc;

    auto maybeSig = sig_finder::findSignature(gs, method);
    if (!maybeSig.has_value()) {
        return std::nullopt;
    }
    core::LocOffsets sigLoc = {maybeSig->sig.beginPos(), maybeSig->body.endPos()};

    return make_pair(sigLoc, methodLoc);
}

unique_ptr<string> copyMethodSource(const core::GlobalState &gs, const core::LocOffsets &sigLoc, const core::LocOffsets &methodLoc, const core::FileRef &fref) {

    auto cutSource = [&](core::LocOffsets loc) { return fref.data(gs).source().substr(loc.beginPos(), loc.endPos() - loc.beginPos()); };


    auto sigSource = cutSource(sigLoc);
    auto methodSource = cutSource(methodLoc);
    return make_unique<string>(absl::StrCat(sigSource, "\n  ",  methodSource));
}

vector<unique_ptr<TextDocumentEdit>> getMoveMethodEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                                        const core::lsp::DefinitionResponse *definition, LSPTypecheckerDelegate &typechecker) {

    ENFORCE(definition->symbol.isMethod());


    auto moduleStart = "module NewModule\n  extend T::Sig\n  ";
    auto moduleEnd = "\nend";

    auto fref = definition->termLoc.file();
    auto &file = fref.data(gs);


    auto trees = typechecker.getResolved({fref});
    auto &rootTree = trees[0].tree;
    auto beginLoc = core::Loc::offset2Pos(fref.data(gs), rootTree.loc().beginPos());

    auto pos2Loc = [&](const auto &pos) {
        auto loc = core::Loc::offset2Pos(fref.data(gs), pos);
        return make_unique<Position>(loc.line-1, loc.column-1);
    };

    auto range2Loc = [&](const auto &pos) {
        return make_unique<Range>(pos2Loc(pos.beginPos()), pos2Loc(pos.endPos()));
    };

    auto topOfTheFile = file.getLine(beginLoc.line);
    auto sigAndMethodLocs = methodLocs(gs, rootTree, definition->symbol, fref);
    if (!sigAndMethodLocs.has_value()) {
        return {};
    }
    auto [sigLoc, methodLoc] = sigAndMethodLocs.value();
    auto methodSource = copyMethodSource(gs, sigLoc, methodLoc, fref);

    auto range = make_unique<Range>(make_unique<Position>(beginLoc.line-1, 0), make_unique<Position>(beginLoc.line-1, topOfTheFile.length()));
    auto replacement = fmt::format("{}{}{}\n\n{}", moduleStart, *methodSource, moduleEnd, topOfTheFile);
    auto moveNewMethod = make_unique<TextEdit>(std::move(range), replacement);
    auto deleteOldSig = make_unique<TextEdit>( range2Loc(sigLoc), "");
    auto deleteOldMethod = make_unique<TextEdit>( range2Loc(methodLoc), "");
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(std::move(moveNewMethod));
    edits.emplace_back(std::move(deleteOldSig));
    edits.emplace_back(std::move(deleteOldMethod));
    auto docEdit = make_unique<TextDocumentEdit>(make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, definition->termLoc.file()), JSONNullObject()), std::move(edits));




    vector<unique_ptr<TextDocumentEdit>> res;
    res.emplace_back(std::move(docEdit));
    return res;
}
} // namespace

unique_ptr<ResponseMessage> CodeActionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    vector<unique_ptr<CodeAction>> result;

    const core::GlobalState &gs = typechecker.state();
    core::FileRef file = config.uri2FileRef(gs, params->textDocument->uri);
    if (!file.exists()) {
        // File is an invalid URI. Perhaps the user opened a file that is not within the VS Code workspace?
        // Don't send an error, as it's not the user's fault and isn't actionable. Instead, send an empty list of code
        // actions.
        response->result = move(result);
        return response;
    }

    // Simply querying the file in question is insufficient since indexing errors would not be detected.
    auto errors = typechecker.retypecheck({file});
    auto loc = params->range->toLoc(gs, file);
    vector<core::AutocorrectSuggestion::Edit> allEdits;
    for (auto &error : errors) {
        if (!error->isSilenced && !error->autocorrects.empty()) {
            // Collect all autocorrects regardless of range to compile into a "source" autocorrect whose scope is
            // the whole file.
            for (auto &autocorrect : error->autocorrects) {
                allEdits.insert(allEdits.end(), autocorrect.edits.begin(), autocorrect.edits.end());
            }

            // We return code actions corresponding to any error that encloses the request's range. Matching request
            // ranges against error ranges exactly prevents VSCode's quick fix shortcut (Cmd+.) from matching any
            // actions since it sends a 0 length range (i.e. the cursor). VSCode's request does include matching
            // diagnostics in the request context that could be used instead but this simpler approach should suffice
            // until proven otherwise.
            if (!error->loc.contains(loc)) {
                continue;
            }

            for (auto &autocorrect : error->autocorrects) {
                auto action = make_unique<CodeAction>(autocorrect.title);
                action->kind = CodeActionKind::Quickfix;
                auto workspaceEdit = make_unique<WorkspaceEdit>();
                workspaceEdit->documentChanges = getEdits(config, gs, autocorrect.edits);
                action->edit = move(workspaceEdit);
                result.emplace_back(move(action));
            }
        }
    }

    // TODO(sushain): investigate where duplicates might happen and whether there is a better fix
    // Remove any actions with the same header regardless of their actions since users cannot make an informed decision
    // between two seemingly identical actions.
    fast_sort(result, [](const auto &l, const auto &r) -> bool { return l->title.compare(r->title) > 0; });
    auto last =
        unique(result.begin(), result.end(), [](const auto &l, const auto &r) -> bool { return l->title == r->title; });
    result.erase(last, result.end());

    // Perform _after_ sorting so these appear at the end of the list.
    if (!allEdits.empty()) {
        // Make a source autocorrect that appears in the "source" menu, and an autocorrect that appears in the
        // quickfix dropdown if there are other quickfixes at this location.
        vector<CodeActionKind> kinds = {CodeActionKind::SourceFixAllSorbet};
        if (!result.empty()) {
            kinds.push_back(CodeActionKind::Quickfix);
        }

        for (auto kind : kinds) {
            auto action = make_unique<CodeAction>("Apply all Sorbet fixes for file");
            action->kind = kind;
            auto workspaceEdit = make_unique<WorkspaceEdit>();
            workspaceEdit->documentChanges = getEdits(config, gs, allEdits);
            action->edit = move(workspaceEdit);
            result.emplace_back(move(action));
        }
    }

    if (config.opts.lspExtractMethodEnabled) {
        auto queryResult = queryByLoc(typechecker, params->textDocument->uri, *params->range->start,
                                      LSPMethod::TextDocumentCodeAction, false);

        // Generate "Extract method" code actions only for method definitions
        if (queryResult.error == nullptr) {
            for (auto &resp : queryResult.responses) {
                if (auto def = resp->isDefinition()) {
                    if (def->symbol.isMethod()) {
                        auto command = make_unique<Command>()
                        auto action = make_unique<CodeAction>("Extract method to module");
                        action->kind = CodeActionKind::RefactorExtract;
                        auto workspaceEdit = make_unique<WorkspaceEdit>();
                        workspaceEdit->documentChanges = getMoveMethodEdits(config, gs, def, typechecker);
                        action->edit = move(workspaceEdit);
                        result.emplace_back(move(action));
                    }
                }
            }
        }
    }

    response->result = move(result);
    return response;
}
} // namespace sorbet::realmain::lsp
