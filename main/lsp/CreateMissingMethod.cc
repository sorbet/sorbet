#include "CreateMissingMethod.h"
#include "ast/treemap/treemap.h"
#include "core/sig_finder/sig_finder.h"
#include "main/lsp/json_types.h"

using namespace std;
namespace sorbet::realmain::lsp {

namespace {

struct MethodDefFinder {
    core::LocOffsets target;
    const ast::MethodDef *result = nullptr;

    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &tree) {
        if (tree.loc.contains(target)) {
            result = &tree;
        }
    }
};

struct SendFinder {
    core::LocOffsets target;
    const ast::Send *result = nullptr;

    void preTransformSend(core::Context ctx, const ast::Send &tree) {
        if (tree.loc.contains(target)) {
            result = &tree;
        }
    }
};

string formatNewMethod(const core::GlobalState &gs, uint32_t indentLength, const core::NameRef name,
                       const vector<string> &paramNames, absl::Span<const core::TypePtr> argTypes, uint64_t numPosArgs,
                       uint64_t numKwArgs) {
    string indent(indentLength, ' ');
    string paramSig = "";
    for (uint64_t i = 0; i < numPosArgs; i++) {
        if (i != 0) {
            paramSig += ", ";
        }
        paramSig +=
            fmt::format("{}: {}", paramNames[i], argTypes[i].show(gs, core::ShowOptions{}.withUseValidSyntax()));
    }
    string paramList = "";
    for (uint64_t i = 0; i < numPosArgs; i++) {
        if (i != 0) {
            paramList += ", ";
        }
        paramList += paramNames[i];
    }
    // for (uint64_t i = 0; i < numKwArgs; i++) {
    //     if (!paramList.empty()) {
    //         paramList += ", ";
    //     }
    //     paramList += fmt::format("{}:", paramNames[i]);
    // }
    string newText =
        fmt::format("\n"
                    "{}sig {{ params({}).returns(T.untyped) }}\n"
                    "{}def {}({})\n"
                    "{}Kernel.raise NotImplementedError\n"
                    "{}end\n",
                    indent, paramSig, indent, name.shortName(gs), paramList, string(indentLength + 2, ' '), indent);
    return newText;
}

optional<core::NameRef> getParamName(const ast::ExpressionPtr &arg) {
    if (auto local = ast::cast_tree<ast::Local>(arg)) {
        return local->localVariable._name;
    } else {
        return nullopt;
    }
}

string getFreshName(UnorderedMap<string, uint32_t> &seen, string_view name) {
    if (seen.contains(name)) {
        seen[name]++;
        return fmt::format("{}{}", name, seen[name]);
    } else {
        seen[name] = 0;
        return string(name);
    }
}

vector<string> getParamNames(const core::GlobalState &gs, const string &defaultName, const ast::Send &send) {
    vector<string> paramNames;
    vector<string> kwParamNames;
    UnorderedMap<string, uint32_t> seen;
    // for (auto [key, _arg] : send.kwArgPairs()) {
    //     auto &lit = cast_tree_nonnull<ast::Literal>(key);
    //     kwParamNames.emplace_back(getFreshName(seen, lit.asString().shortName(gs)));
    // }
    for (auto &arg : send.posArgs()) {
        auto name = getParamName(arg);
        if (name.has_value()) {
            paramNames.emplace_back(getFreshName(seen, name.value().shortName(gs)));
        } else {
            paramNames.emplace_back(getFreshName(seen, defaultName));
        }
    }
    paramNames.insert(paramNames.end(), make_move_iterator(kwParamNames.begin()),
                      make_move_iterator(kwParamNames.end()));
    return paramNames;
}

} // namespace

const core::lsp::SendResponse *isMissingMethodResponse(const core::GlobalState &gs,
                                                       const vector<unique_ptr<core::lsp::QueryResponse>> &responses) {
    if (responses.empty()) {
        return nullptr;
    }

    auto *resp = responses[0]->isSend();
    if (resp == nullptr) {
        return nullptr;
    }
    auto method = resp->dispatchResult->main.method;
    if (method.exists()) {
        return nullptr;
    }

    return resp;
}

vector<unique_ptr<TextDocumentEdit>> getCreateMissingMethodEdits(LSPTypecheckerDelegate &typechecker,

                                                                 const LSPConfiguration &config,
                                                                 const core::lsp::SendResponse &resp) {
    auto &gs = typechecker.state();
    auto file = resp.file;
    auto resolvedTree = typechecker.getResolved(file);
    auto &rootTree = resolvedTree.tree;

    auto enclosingMethodRef = resp.enclosingMethod;
    auto enclosingMethod = enclosingMethodRef.data(gs);
    auto enclosingMethodDeclLoc = enclosingMethod->loc();
    if (!enclosingMethodRef.exists()) {
        // TODO(bshu) handle this case
        return {};
    }
    auto ctx = core::Context(gs, core::Symbols::root(), file);
    MethodDefFinder finder{enclosingMethodDeclLoc.offsets()};
    ast::ConstTreeWalk::apply(ctx, finder, rootTree);
    if (finder.result == nullptr) {
        return {};
    }
    auto enclosingMethodLoc = core::Loc(file, finder.result->loc);
    auto [_, enclosingMethodEnd] = enclosingMethodLoc.toDetails(gs);
    auto insertDetail = core::Loc::Detail{enclosingMethodEnd.line + 1, 1};
    auto insertLoc = core::Loc::fromDetails(gs, enclosingMethodLoc.file(), insertDetail, insertDetail);
    if (!insertLoc.has_value()) {
        return {};
    }
    auto [_loc, indentLength] = enclosingMethodLoc.copyEndWithZeroLength().findStartOfIndentation(gs);

    SendFinder sendFinder{resp.funLocOffsets};
    ast::ConstTreeWalk::apply(ctx, sendFinder, rootTree);
    if (sendFinder.result == nullptr) {
        return {};
    }

    auto paramNames = getParamNames(gs, "param", *sendFinder.result);
    auto newText =
        formatNewMethod(gs, indentLength, resp.originalName, paramNames, resp.argTypes, resp.numPosArgs, resp.numArgs);
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, insertLoc.value()), newText));

    auto tdi = make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, enclosingMethodLoc.file()),
                                                            JSONNullObject());
    vector<unique_ptr<TextDocumentEdit>> result;
    result.emplace_back(make_unique<TextDocumentEdit>(move(tdi), move(edits)));
    return result;
}
} // namespace sorbet::realmain::lsp
