#include "main/lsp/MoveMethod.h"
#include "main/lsp/AbstractRewriter.h"
#include "main/sig_finder/sig_finder.h"
using namespace std;

namespace sorbet::realmain::lsp {

namespace {

const auto moduleKeyword = "module "s;

bool isTSigRequired(const core::GlobalState &gs) {
    return !core::Symbols::Module().data(gs)->derivesFrom(gs, core::Symbols::T_Sig());
}

optional<const ast::MethodDef *> findMethodTree(const ast::ExpressionPtr &tree, const core::SymbolRef method) {
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
    return nullopt;
}

optional<pair<optional<core::LocOffsets>, core::LocOffsets>> methodLocs(const core::GlobalState &gs,
                                                                        ast::ExpressionPtr &rootTree,
                                                                        const core::MethodRef method,
                                                                        const core::FileRef fref) {
    auto maybeTree = findMethodTree(rootTree, method);
    if (!maybeTree.has_value()) {
        return nullopt;
    }
    auto methodLoc = maybeTree.value()->loc;

    auto ctx = core::Context(gs, core::Symbols::root(), fref);
    auto queryLoc = ctx.locAt(methodLoc.copyWithZeroLength());
    auto parsedSig = sig_finder::SigFinder::findSignature(ctx, rootTree, queryLoc);
    if (!parsedSig.has_value()) {
        return make_pair(nullopt, methodLoc);
    }
    return make_pair(parsedSig->origSend->loc, methodLoc);
}

// Turns ruby_function_name__ to RubyFunctionName,
// so we can use it in a new module name
string snakeToCamelCase(string_view name) {
    string res;
    const auto originalSize = name.size();
    res.reserve(originalSize);
    bool shouldCapitalize = true;
    for (int i = 0; i < originalSize; i++) {
        if (name.at(i) == '_') {
            shouldCapitalize = true;
        }
        if (!isalnum(name.at(i))) {
            continue;
        }
        res += shouldCapitalize ? toupper(name[i]) : name[i];
        shouldCapitalize = false;
    }
    return res;
}

optional<string> getNewModuleName(const core::GlobalState &gs, const core::NameRef name) {
    const auto moduleName = absl::StrCat(snakeToCamelCase(name.show(gs)), "Module");
    const auto nameNotExists = [&](auto moduleName) {
        if (auto nameRef = gs.lookupNameUTF8(moduleName); !nameRef.exists()) {
            // We're using root here, because newely defined module would be at the top level
            return !core::Symbols::root().data(gs)->findMember(gs, nameRef).exists();
        }
        return false;
    };

    // if there are no names like that, return the new module name
    if (nameNotExists(moduleName)) {
        return moduleName;
    }

    // otherwise, try to augment it
    for (int i = 1; i < 42; i++) {
        const auto newName = absl::StrCat(moduleName, i);
        if (nameNotExists(newName)) {
            return newName;
        }
    }

    // bail, if we haven't found untaken name in 42 tries
    return nullopt;
}

class MethodCallSiteRenamer : public AbstractRewriter {
    string oldName;
    string newName;

public:
    MethodCallSiteRenamer(const core::GlobalState &gs, const LSPConfiguration &config, const string oldName,
                          const string newName)
        : AbstractRewriter(gs, config), oldName(oldName), newName(newName) {
        const vector<string> invalidNames = {"initialize", "call"};
        for (auto name : invalidNames) {
            if (oldName == name) {
                invalid = true;
                error = fmt::format("The `{}` method cannot be moved to a module.", oldName);
                return;
            }
        }
    }

    ~MethodCallSiteRenamer() {}
    void rename(unique_ptr<core::lsp::QueryResponse> &response, const core::SymbolRef originalSymbol) override {
        if (invalid) {
            return;
        }
        auto loc = response->getLoc();

        // If we're renaming the exact same place twice, silently ignore it. We reach this condition when we find the
        // same method send through multiple definitions (e.g. in the case of union types)
        auto it = edits.find(loc);
        if (it != edits.end()) {
            return;
        }

        auto source = loc.source(gs);
        if (!source.has_value()) {
            return;
        }
        if (auto sendResp = response->isSend()) {
            // if the call site is not trivial, don't attempt to rename
            // the typecheck error will guide user how to fix it
            for (auto dr = sendResp->dispatchResult.get(); dr != nullptr; dr = dr->secondary.get()) {
                if (dr->main.method != originalSymbol.asMethodRef()) {
                    return;
                }
            }

            edits[sendResp->receiverLoc()] = newName;
        }
    }
    void addSymbol(const core::SymbolRef symbol) override {
        if (!symbol.isMethod()) {
            return;
        }
        addSubclassRelatedMethods(gs, symbol.asMethodRef(), getQueue());
    }
}; // MethodCallSiteRenamer

vector<unique_ptr<TextEdit>> moveMethod(LSPTypecheckerDelegate &typechecker, const LSPConfiguration &config,
                                        const core::lsp::MethodDefResponse &definition, string_view newModuleName) {
    auto &gs = typechecker.state();
    auto moduleStart =
        fmt::format("{}{}{}\n  ", moduleKeyword, newModuleName, isTSigRequired(gs) ? "\n  extend T::Sig" : "");
    auto moduleEnd = "\nend";

    auto fref = definition.termLoc.file();

    auto trees = typechecker.getResolved({fref});
    ENFORCE(!trees.empty());
    auto &rootTree = trees[0].tree;

    auto sigAndMethodLocs = methodLocs(gs, rootTree, definition.symbol, fref);
    if (!sigAndMethodLocs.has_value()) {
        return {};
    }
    auto [maybeSigLoc, methodLoc] = sigAndMethodLocs.value();
    auto methodSourceLocOffset = maybeSigLoc.has_value() ? maybeSigLoc.value().join(methodLoc) : methodLoc;
    auto methodSourceLoc = core::Loc(fref, methodSourceLocOffset);
    auto methodSource = methodSourceLoc.source(gs);
    if (!methodSource.has_value()) {
        return {};
    }

    auto insertPosition = Range::fromLoc(gs, core::Loc(fref, rootTree.loc().copyWithZeroLength()));
    auto newModuleSource = fmt::format("{}{}{}\n\n", moduleStart, methodSource.value(), moduleEnd);

    // This manipulations with the positions are required to remove leading tabs and whitespaces at the original method
    // position
    auto [oldMethodStart, oldMethodEnd] = methodSourceLoc.position(gs);
    auto oldMethodLoc = core::Loc::fromDetails(gs, fref, {oldMethodStart.line, 0}, oldMethodEnd);
    ENFORCE(oldMethodLoc.has_value());

    vector<unique_ptr<TextEdit>> res;
    res.emplace_back(make_unique<TextEdit>(move(insertPosition), newModuleSource));
    res.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, oldMethodLoc.value()), ""));

    return res;
}

} // namespace

unique_ptr<Position> getNewModuleLocation(const core::GlobalState &gs, const core::lsp::MethodDefResponse &definition,
                                          LSPTypecheckerDelegate &typechecker) {
    auto fref = definition.termLoc.file();

    auto &rootTree = typechecker.getIndexed({fref});
    auto insertPosition = Range::fromLoc(gs, core::Loc(fref, rootTree.tree.loc().copyWithZeroLength()));
    auto newModuleSymbol = insertPosition->start->copy();
    newModuleSymbol->character += moduleKeyword.size() + 1;
    return newModuleSymbol;
}

vector<unique_ptr<TextDocumentEdit>> getMoveMethodEdits(LSPTypecheckerDelegate &typechecker,
                                                        const LSPConfiguration &config,
                                                        const core::lsp::MethodDefResponse &definition) {
    vector<unique_ptr<TextDocumentEdit>> res;
    auto &gs = typechecker.state();
    auto newModuleName = getNewModuleName(gs, definition.name);
    if (!newModuleName.has_value()) {
        return res;
    }

    auto edits = moveMethod(typechecker, config, definition, newModuleName.value());

    auto renamer = make_shared<MethodCallSiteRenamer>(gs, config, definition.name.show(gs), newModuleName.value());
    renamer->getEdits(typechecker, definition.symbol);
    auto callSiteEdits = renamer->buildTextDocumentEdits();

    if (callSiteEdits.has_value()) {
        for (auto &edit : callSiteEdits.value()) {
            res.emplace_back(move(edit));
        }
    }
    auto docEdit =
        make_unique<TextDocumentEdit>(make_unique<VersionedTextDocumentIdentifier>(
                                          config.fileRef2Uri(gs, definition.termLoc.file()), JSONNullObject()),
                                      move(edits));

    res.emplace_back(move(docEdit));
    return res;
}
} // namespace sorbet::realmain::lsp
