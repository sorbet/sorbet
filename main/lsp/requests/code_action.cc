#include "main/lsp/requests/code_action.h"
#include "absl/algorithm/container.h"
#include "common/sort.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ConvertToSingletonClassMethod.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/MoveMethod.h"
#include "main/lsp/json_types.h"
#include "main/sig_finder/sig_finder.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
const UnorderedSet<string> OPERATORS = {"+",  "−",  "*",   "/",   "%",   "**",    "==",     "!=",  ">",
                                        "<",  ">=", "<=",  "<=>", "===", ".eql?", "equal?", "=",   "+=",
                                        "-=", "*=", "/=",  "%=",  "**=", "&",     "|",      "^",   "~",
                                        "<<", ">>", "and", "or",  "&&",  "||",    "!",      "not", ".."};

bool isOperator(string_view name) {
    return OPERATORS.contains(name);
}

vector<unique_ptr<TextDocumentEdit>> getQuickfixEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                                      const vector<core::AutocorrectSuggestion::Edit> &edits) {
    UnorderedMap<string, vector<unique_ptr<TextEdit>>> editsByFile;
    for (auto &edit : edits) {
        auto range = Range::fromLoc(gs, edit.loc);
        if (range != nullptr) {
            editsByFile[config.fileRef2Uri(gs, edit.loc.file())].emplace_back(
                make_unique<TextEdit>(move(range), edit.replacement));
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

const core::lsp::MethodDefResponse *
hasLoneMethodResponse(const core::GlobalState &gs, const vector<unique_ptr<core::lsp::QueryResponse>> &responses) {
    // We want to return the singular `MethodDefResponse` for a non-operator method.
    // We do not want to return the first such response, because there might be multiple
    // methods "defined" at the same location (cf. the DSLBuilder rewriter).  And because the
    // query we're examining was a location-based query, there might be several overlapping responses,
    // and we only want to consider the method ones.
    const core::lsp::MethodDefResponse *found = nullptr;

    for (auto &resp : responses) {
        if (auto *def = resp->isMethodDef()) {
            if (found != nullptr) {
                // Assume these two methods stem from some sort of rewriter pass.
                return nullptr;
            }

            // If we find a method def we can't handle at this location, assume
            // it also comes from sort of rewriter pass.
            if (isOperator(def->name.show(gs))) {
                return nullptr;
            }

            found = def;
        }
    }

    return found;
}
} // namespace

CodeActionTask::CodeActionTask(const LSPConfiguration &config, MessageId id, unique_ptr<CodeActionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentCodeAction), params(move(params)) {}

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

    auto maybeLoc = params->range->toLoc(gs, file);
    if (!maybeLoc.has_value()) {
        // VSCode has been observed to send bad ranges for rubocop autofixes.  It's
        // not clear whose fault that is, but we shouldn't send an error for that.
        response->result = move(result);
        return response;
    }
    auto loc = maybeLoc.value();
    // Simply querying the file in question is insufficient since indexing errors would not be detected.
    auto errors = typechecker.retypecheck({file});
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
                workspaceEdit->documentChanges = getQuickfixEdits(config, gs, autocorrect.edits);
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
            workspaceEdit->documentChanges = getQuickfixEdits(config, gs, allEdits);
            action->edit = move(workspaceEdit);
            result.emplace_back(move(action));
        }
    }

    auto queryResult = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->range->start,
                                       LSPMethod::TextDocumentCodeAction, false);

    // Generate "Move method" code actions only for class method definitions
    if (queryResult.error == nullptr) {
        if (auto *def = hasLoneMethodResponse(gs, queryResult.responses)) {
            unique_ptr<CodeAction> action;
            bool canResolveLazily = config.getClientConfig().clientCodeActionResolveEditSupport &&
                                    config.getClientConfig().clientCodeActionDataSupport;

            if (def->symbol.data(gs)->owner.data(gs)->isSingletonClass(gs)) {
                action = make_unique<CodeAction>("Move method to a new module");
                action->kind = CodeActionKind::RefactorExtract;

                auto newModuleLoc = getNewModuleLocation(gs, *def, typechecker);
                auto renameCommand = make_unique<Command>("Rename Symbol", "sorbet.rename");
                auto arg = make_unique<TextDocumentPositionParams>(
                    make_unique<TextDocumentIdentifier>(params->textDocument->uri), move(newModuleLoc));
                auto args = vector<unique_ptr<TextDocumentPositionParams>>();
                args.emplace_back(move(arg));

                // TODO(jez) Do we need the command to be here if it's not lazy?
                renameCommand->arguments = move(args);
                action->command = move(renameCommand);
                if (canResolveLazily) {
                    action->data = move(params);
                } else {
                    auto workspaceEdit = make_unique<WorkspaceEdit>();
                    auto edits = getMoveMethodEdits(typechecker, config, *def);
                    workspaceEdit->documentChanges = move(edits);
                    action->edit = move(workspaceEdit);
                }
            } else {
                action = make_unique<CodeAction>("Convert to singleton class method");
                action->kind = CodeActionKind::RefactorRewrite;

                // TODO(jez) Also support can resolve lazily
                auto renameCommand = make_unique<Command>("Convert to singleton class method",
                                                          "sorbet.convert_to_singleton_class_method");

                auto workspaceEdit = make_unique<WorkspaceEdit>();
                auto edits = convertToSingletonClassMethod(typechecker, config, *def);
                workspaceEdit->documentChanges = move(edits);
                action->edit = move(workspaceEdit);
            }

            result.emplace_back(move(action));
        }
    }

    response->result = move(result);
    return response;
}

} // namespace sorbet::realmain::lsp
