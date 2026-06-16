#include "CreateMissingMethod.h"
#include "ast/treemap/treemap.h"
#include "core/sig_finder/sig_finder.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {

void collectSelfTypeParams(const core::TypePtr &type, UnorderedSet<core::SymbolRef> &result) {
    switch (type.tag()) {
        case core::TypePtr::Tag::ClassType:
        case core::TypePtr::Tag::BlamedUntyped:
        case core::TypePtr::Tag::UnresolvedClassType:
        case core::TypePtr::Tag::UnresolvedAppliedType:
        case core::TypePtr::Tag::LambdaParam:
        case core::TypePtr::Tag::AliasType:
        case core::TypePtr::Tag::SelfType:
        case core::TypePtr::Tag::IntegerLiteralType:
        case core::TypePtr::Tag::FloatLiteralType:
        case core::TypePtr::Tag::NamedLiteralType:
        case core::TypePtr::Tag::TypeVar:
        case core::TypePtr::Tag::MetaType:
            return;
        case core::TypePtr::Tag::SelfTypeParam: {
            auto def = cast_type_nonnull<core::SelfTypeParam>(type).definition;
            if (def.isTypeMember()) {
                return;
            }
            result.insert(def);
            return;
        }
        case core::TypePtr::Tag::OrType: {
            auto &o = cast_type_nonnull<core::OrType>(type);
            collectSelfTypeParams(o.left, result);
            collectSelfTypeParams(o.right, result);
            return;
        }
        case core::TypePtr::Tag::AndType: {
            auto &a = cast_type_nonnull<core::AndType>(type);
            collectSelfTypeParams(a.left, result);
            collectSelfTypeParams(a.right, result);
            return;
        }
        case core::TypePtr::Tag::AppliedType: {
            auto &app = cast_type_nonnull<core::AppliedType>(type);
            for (auto &targ : app.targs) {
                collectSelfTypeParams(targ, result);
            }
            return;
        }
        case core::TypePtr::Tag::TupleType: {
            auto &tuple = cast_type_nonnull<core::TupleType>(type);
            for (auto &elem : tuple.elems) {
                collectSelfTypeParams(elem, result);
            }
            return;
        }
        case core::TypePtr::Tag::ShapeType: {
            auto &shape = cast_type_nonnull<core::ShapeType>(type);
            for (auto &val : shape.values) {
                collectSelfTypeParams(val, result);
            }
            return;
        }
    }
}

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

core::TypePtr improveArgType(const core::GlobalState &gs, const core::TypePtr &argType) {
    if (isa_type<core::ClassType>(argType)) {
        auto ct = core::cast_type_nonnull<core::ClassType>(argType);
        if (ct.symbol == core::Symbols::TrueClass() || ct.symbol == core::Symbols::FalseClass()) {
            return core::Types::Boolean();
        }
    }
    return argType;
}

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
    for (int i = 0; i < numKwArgs; i++) {
        if (!paramSig.empty()) {
            paramSig += ", ";
        }
        paramSig += fmt::format("{}: {}", paramNames[numPosArgs + i],
                                argTypes[numPosArgs + i * 2 + 1].show(gs, core::ShowOptions{}.withUseValidSyntax()));
    }
    string paramList = "";
    for (uint64_t i = 0; i < numPosArgs; i++) {
        if (i != 0) {
            paramList += ", ";
        }
        paramList += paramNames[i];
    }
    for (uint64_t i = 0; i < numKwArgs; i++) {
        if (!paramList.empty()) {
            paramList += ", ";
        }
        paramList += fmt::format("{}:", paramNames[numPosArgs + i]);
    }
    UnorderedSet<core::SymbolRef> selfTypeParams;
    for (auto &argType : argTypes) {
        collectSelfTypeParams(argType, selfTypeParams);
    }
    string typeParams = "";
    if (!selfTypeParams.empty()) {
        typeParams += "T.type_parameters(";
        bool first = true;
        for (auto &param : selfTypeParams) {
            if (!first) {
                typeParams += ", ";
            }
            first = false;
            typeParams += fmt::format(":{}", param.name(gs).show(gs));
        }
        typeParams += ").";
    }
    string newText = fmt::format("\n"
                                 "\n"
                                 "{}sig {{ {}params({}).returns(T.untyped) }}\n"
                                 "{}def {}({})\n"
                                 "{}  Kernel.raise NotImplementedError\n"
                                 "{}end",
                                 indent, typeParams, paramSig, indent, name.shortName(gs), paramList, indent, indent);
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
    for (auto [key, _arg] : send.kwArgPairs()) {
        auto &lit = cast_tree_nonnull<ast::Literal>(key);
        auto kwName = lit.asSymbol();
        kwParamNames.emplace_back(getFreshName(seen, kwName.shortName(gs)));
    }
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

namespace create_missing_method {

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
    // the enclosing method always exists
    auto enclosingMethod = enclosingMethodRef.data(gs);
    auto enclosingMethodDeclLoc = enclosingMethod->loc();
    auto ctx = core::Context(gs, core::Symbols::root(), file);
    MethodDefFinder finder{enclosingMethodDeclLoc.offsets()};
    ast::ConstTreeWalk::apply(ctx, finder, rootTree);
    if (finder.result == nullptr) {
        return {};
    }
    auto enclosingMethodLoc = core::Loc(file, finder.result->loc);
    auto insertLoc = enclosingMethodLoc.copyEndWithZeroLength();
    auto [_loc, indentLength] = enclosingMethodLoc.copyEndWithZeroLength().findStartOfIndentation(gs);

    SendFinder sendFinder{resp.funLocOffsets};
    ast::ConstTreeWalk::apply(ctx, sendFinder, rootTree);
    if (sendFinder.result == nullptr) {
        return {};
    }

    auto paramNames = getParamNames(gs, "param", *sendFinder.result);
    vector<core::TypePtr> improvedArgTypes;
    for (auto &argType : resp.argTypes) {
        improvedArgTypes.emplace_back(improveArgType(gs, argType));
    }
    auto newText = formatNewMethod(gs, indentLength, resp.originalName, paramNames, improvedArgTypes,
                                   sendFinder.result->numPosArgs(), sendFinder.result->numKwArgs());
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, insertLoc), newText));

    auto tdi = make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, enclosingMethodLoc.file()),
                                                            JSONNullObject());
    vector<unique_ptr<TextDocumentEdit>> result;
    result.emplace_back(make_unique<TextDocumentEdit>(move(tdi), move(edits)));
    return result;
}

} // namespace create_missing_method

} // namespace sorbet::realmain::lsp
