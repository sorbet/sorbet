#include "main/autogen/token_prop_analysis.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "main/autogen/crc_builder.h"

using namespace std;
namespace sorbet::autogen {

const std::vector<uint32_t> KNOWN_PROP_METHODS = {
    core::Names::tokenProp().rawId(), core::Names::timestampedTokenProp().rawId(),
    core::Names::registerPrefix().rawId(), core::Names::setArchiveTokenPrefix().rawId()};

const std::vector<core::NameRef> ABSTRACT_BLACKLIST_RECORD = {
    core::Names::Constants::Opus(),
    core::Names::Constants::Risk(),
    core::Names::Constants::Denylists(),
    core::Names::Constants::Model(),
    core::Names::Constants::AbstractBlacklistRecord(),
};

class TokenPropAnalysisWalk {
    UnorderedMap<vector<core::NameRef>, TokenProps> tokenPropsByClass;
    vector<vector<core::NameRef>> nestingScopes;
    core::FileRef file;
    bool validScope;

    // Convert a symbol name into a fully qualified name
    vector<core::NameRef> symbolName(core::Context ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.name(ctx));
            sym = sym.owner(ctx);
        }
        reverse(out.begin(), out.end());
        return out;
    }

    struct PropInfoInternal {
        core::NameRef name;
        bool isTimestamped;
    };

    std::optional<PropInfoInternal> parseProp(core::Context ctx, ast::Send *send) {
        switch (send->fun.rawId()) {
            case core::Names::timestampedTokenProp().rawId():
                if (send->numPosArgs() > 0) {
                    auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(0));
                    if (lit && lit->isString()) {
                        return PropInfoInternal{lit->asString(), true};
                    } else {
                        return PropInfoInternal{core::NameRef::noName(), true};
                    }
                }
                break;
            case core::Names::setArchiveTokenPrefix().rawId():
            case core::Names::registerPrefix().rawId():
            case core::Names::tokenProp().rawId():
                if (send->numPosArgs() > 0) {
                    auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(0));
                    if (lit && lit->isString()) {
                        return PropInfoInternal{lit->asString(), false};
                    } else {
                        return PropInfoInternal{core::NameRef::noName(), false};
                    }
                }
                break;
            default:
                return std::nullopt;
        }

        return std::nullopt;
    }

public:
    TokenPropAnalysisWalk(core::FileRef a_file) {
        validScope = true;
        file = a_file;
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (original.symbol.data(ctx)->owner == core::Symbols::PackageSpecRegistry()) {
            // this is a package, so do not enter a definition for it
            return;
        }

        vector<vector<core::NameRef>> ancestors;
        for (auto &ancst : original.ancestors) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
            if (cnst == nullptr || cnst->original == nullptr) {
                // ignore them if they're not statically-known ancestors (i.e. not constants)
                continue;
            }

            const auto ancstName = symbolName(ctx, cnst->symbol);
            ancestors.emplace_back(std::move(ancstName));
        }

        const vector<core::NameRef> className = symbolName(ctx, original.symbol);
        nestingScopes.emplace_back(className);
        tokenPropsByClass.emplace(className, TokenProps{{}, ancestors, file});

        return;
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not in any scope
            return;
        }

        nestingScopes.pop_back();

        return;
    }

    void preTransformSend(core::Context ctx, ast::ExpressionPtr &tree) {
        if (nestingScopes.size() == 0) {
            // Not in any scope
            return;
        }

        auto *original = ast::cast_tree<ast::Send>(tree);
        auto &curScope = nestingScopes.back();

        uint32_t funId = original->fun.rawId();
        bool isProp = absl::c_any_of(KNOWN_PROP_METHODS, [&](const auto &nrid) -> bool { return nrid == funId; });
        if (isProp) {
            if (!validScope) {
                return;
            }

            const auto prop = parseProp(ctx, original);
            if (prop.has_value()) {
                tokenPropsByClass[curScope].props.emplace_back(
                    PropInfo{std::move((*prop).name), std::move((*prop).isTimestamped)});
            } else {
                tokenPropsByClass[curScope].props.emplace_back(PropInfo{core::NameRef::noName(), false});
            }

            return;
        }

        return;
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        if (nestingScopes.size() == 0 || !validScope) {
            // Not already in a valid scope
            return;
        }

        auto &curScope = nestingScopes.back();
        if (curScope == ABSTRACT_BLACKLIST_RECORD) {
            return;
        }

        validScope = false;
        return;
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        if (nestingScopes.size() == 0 || validScope) {
            // Already in a valid scope, or never in a scope
            return;
        }

        validScope = true;
        return;
    }

    TokenPropAnalysisFile tokenPropAnalysisFile() {
        TokenPropAnalysisFile out;
        out.tokenPropsByClass = std::move(tokenPropsByClass);
        out.file = std::move(file);
        return out;
    }
};

TokenPropAnalysisFile TokenPropAnalysis::generate(core::Context ctx, ast::ParsedFile tree,
                                                  const CRCBuilder &crcBuilder) {
    TokenPropAnalysisWalk walk(tree.file);
    ast::TreeWalk::apply(ctx, walk, tree.tree);
    auto tpaf = walk.tokenPropAnalysisFile();
    auto src = tree.file.data(ctx).source();
    tpaf.cksum = crcBuilder.crc32(src);
    return tpaf;
}

} // namespace sorbet::autogen
