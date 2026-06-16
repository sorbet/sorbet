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

struct ClassDefFinder {
    core::ClassOrModuleRef target;
    const ast::ClassDef *result = nullptr;

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &tree) {
        if (tree.symbol == target) {
            result = &tree;
        }
    }
};

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

string formatNewMethod(const core::GlobalState &gs, uint32_t indentLength, const bool singletonClass,
                       const core::NameRef name, const vector<string> &paramNames,
                       absl::Span<const core::TypePtr> argTypes, uint64_t numPosArgs, uint64_t numKwArgs) {
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
    string singletonClassPrefix = singletonClass ? "self." : "";
    string newText = fmt::format("\n"
                                 "{}sig {{ {}params({}).returns(T.untyped) }}\n"
                                 "{}def {}{}({})\n"
                                 "{}  Kernel.raise NotImplementedError\n"
                                 "{}end",
                                 indent, typeParams, paramSig, indent, singletonClassPrefix, name.shortName(gs),
                                 paramList, indent, indent);
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

core::ClassOrModuleRef getClass(const core::GlobalState &gs, const core::TypePtr &type) {
    if (isa_type<core::ClassType>(type)) {
        auto ct = cast_type_nonnull<core::ClassType>(type);
        return ct.symbol;
    } else if (isa_type<core::AppliedType>(type)) {
        auto &ct = cast_type_nonnull<core::AppliedType>(type);
        return ct.klass;
    }
    return core::Symbols::noClassOrModule();
}

pair<core::Loc, int> getInsertionLocationForClass(LSPTypecheckerDelegate &typechecker, const core::FileRef &currentFile,
                                                  const ast::ParsedFile &currentAst,
                                                  const core::ClassOrModuleRef &classRef) {
    auto &gs = typechecker.state();
    auto classLocs = classRef.data(gs)->locs();
    auto inCurrentFile = [&](const auto &loc) { return loc.file() == currentFile; };
    auto classLocIt = absl::c_find_if(classLocs, inCurrentFile);
    auto insertFile = (classLocIt != classLocs.end()) ? classLocIt->file() : classRef.data(gs)->loc().file();
    ast::ParsedFile insertTreeStorage;
    const ast::ParsedFile *insertTree;
    if (classLocIt != classLocs.end()) {
        insertTreeStorage = typechecker.getResolved(insertFile);
        insertTree = &insertTreeStorage;
    } else {
        insertTree = &currentAst;
    }
    auto classCtx = core::Context(gs, core::Symbols::root(), insertFile);
    ClassDefFinder classFinder{classRef};
    ast::ConstTreeWalk::apply(classCtx, classFinder, insertTree->tree);
    ENFORCE(classFinder.result != nullptr);
    auto classLoc = core::Loc(insertFile, classFinder.result->loc);
    // skip past the `end` keyword
    auto insertLoc = classLoc.copyEndWithZeroLength().adjust(gs, -3, -3);
    auto [_loc, indentLength] = classLoc.copyEndWithZeroLength().findStartOfIndentation(gs);
    return {insertLoc, indentLength};
}

pair<core::Loc, int> getInsertionLocationAfterMethod(const core::GlobalState &gs, const ast::ParsedFile &rootTree,
                                                     const core::Loc &enclosingMethodDeclLoc) {
    auto ctx = core::Context(gs, core::Symbols::root(), rootTree.file);
    MethodDefFinder finder{enclosingMethodDeclLoc.offsets()};
    ast::ConstTreeWalk::apply(ctx, finder, rootTree.tree);
    ENFORCE(finder.result != nullptr);
    auto enclosingMethodLoc = core::Loc(rootTree.file, finder.result->loc);
    auto insertLoc1 = enclosingMethodLoc.copyEndWithZeroLength();
    auto [_loc, indentLength1] = enclosingMethodLoc.copyEndWithZeroLength().findStartOfIndentation(gs);
    return {insertLoc1, indentLength1};
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

    core::Loc insertLoc;
    int indentLength;
    bool singletonClass;
    string extraTextBefore;
    string extraTextAfter;
    auto receiverClass = getClass(gs, resp.dispatchResult->main.receiver);
    if (receiverClass == core::Symbols::noClassOrModule()) {
        return {};
    }
    auto runInsertAfter = [&](bool isSingletonClass) {
        auto [insertLoc1, indentLength1] = getInsertionLocationAfterMethod(gs, resolvedTree, enclosingMethodDeclLoc);
        insertLoc = insertLoc1;
        indentLength = indentLength1;
        singletonClass = isSingletonClass;
        extraTextBefore = "\n";
        extraTextAfter = "";
    };
    auto runCalculateInsert = [&](bool isSingletonClass) {
        auto attachedClassRef = receiverClass.data(gs)->attachedClass(gs);
        auto classRef = isSingletonClass ? attachedClassRef : receiverClass;
        auto [insertLoc1, indentLength1] = getInsertionLocationForClass(typechecker, file, resolvedTree, classRef);
        insertLoc = insertLoc1;
        indentLength = indentLength1 + 2;
        singletonClass = isSingletonClass;
        extraTextBefore = "";
        extraTextAfter = "\n";
    };
    // try to insert the missing method near the enclosing method
    if (receiverClass.data(gs)->isSingletonClass(gs)) {
        if (receiverClass.data(gs)->attachedClass(gs) == enclosingMethodRef.data(gs)->owner) {
            runInsertAfter(true);
        } else {
            runCalculateInsert(true);
        }
    } else {
        if (receiverClass == enclosingMethodRef.data(gs)->owner) {
            runInsertAfter(false);
        } else {
            runCalculateInsert(false);
        }
    }

    auto ctx = core::Context(gs, core::Symbols::root(), file);
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
    auto newText = formatNewMethod(gs, indentLength, singletonClass, resp.originalName, paramNames, improvedArgTypes,
                                   sendFinder.result->numPosArgs(), sendFinder.result->numKwArgs());
    newText = fmt::format("{}{}{}", extraTextBefore, newText, extraTextAfter);
    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, insertLoc), newText));

    auto tdi = make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, insertLoc.file()), JSONNullObject());
    vector<unique_ptr<TextDocumentEdit>> result;
    result.emplace_back(make_unique<TextDocumentEdit>(move(tdi), move(edits)));
    return result;
}

} // namespace create_missing_method

} // namespace sorbet::realmain::lsp
