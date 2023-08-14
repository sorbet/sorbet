#ifndef SORBET_SIG_FINDER
#define SORBET_SIG_FINDER
#include "core/core.h"
#include "resolver/type_syntax/type_syntax.h"

namespace sorbet::sig_finder {

class SigFinder {
    const core::Loc queryLoc;

    // Track the narrowest location range that still contains the queryLoc.
    //
    // If we find a method that's after queryLoc but it's not in this narrowest range,
    // it means we found a sig that's outside the scope where the queryLoc was.
    core::Loc narrowestClassDefRange;

    // Track whether the current scope has the queryLoc.
    std::vector<bool> scopeContainsQueryLoc;

    std::optional<resolver::ParsedSig> result_;

public:
    SigFinder(core::Loc queryLoc)
        : queryLoc(queryLoc), narrowestClassDefRange(core::Loc::none()), scopeContainsQueryLoc(std::vector<bool>{}),
          result_(std::nullopt) {}

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree);
    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree);
    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree);
    void postTransformRuntimeMethodDefinition(core::Context ctx, ast::ExpressionPtr &tree);
    void preTransformSend(core::Context ctx, ast::ExpressionPtr &tree);

    static std::optional<resolver::ParsedSig> findSignature(core::Context ctx, ast::ExpressionPtr &tree,
                                                            core::Loc queryLoc);
};

} // namespace sorbet::sig_finder

#endif // SORBET_SIG_FINDER
