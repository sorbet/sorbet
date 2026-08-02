#ifndef SORBET_SIG_FINDER
#define SORBET_SIG_FINDER
#include "core/core.h"
#include "resolver/type_syntax/type_syntax.h"

namespace sorbet::sig_finder {

class SigFinder {
public:
    struct Result {
        resolver::ParsedSig sig;
        const ast::Send &origSend;
        Result(resolver::ParsedSig &&sig, const ast::Send &origSend) : sig(std::move(sig)), origSend(origSend) {}
    };

private:
    const core::Loc queryLoc;

    // Track the narrowest location range that still contains the queryLoc.
    //
    // If we find a method that's after queryLoc but it's not in this narrowest range,
    // it means we found a sig that's outside the scope where the queryLoc was.
    core::Loc narrowestClassDefRange;

    // Track whether the current scope has the queryLoc.
    std::vector<bool> scopeContainsQueryLoc;

    // The best `sig` send found so far, and the owner it should be parsed in (i.e., a singleton
    // class, for a `sig` on a `self.` method). We deliberately do NOT parse this send as we find
    // it: parsing a `sig` has the side effect of reporting type errors on it, so it must only
    // ever be parsed once, for the single winning candidate, and only if the caller actually asked
    // for the parsed result (see findSignature vs. findSignatureSend below).
    const ast::Send *bestSend_;
    core::SymbolRef bestOwner_;

    std::optional<Result> parseBestSend(core::Context ctx) const;

public:
    SigFinder(core::Loc queryLoc)
        : queryLoc(queryLoc), narrowestClassDefRange(core::Loc::none()), scopeContainsQueryLoc(std::vector<bool>{}),
          bestSend_(nullptr) {}

    void preTransformClassDef(core::Context ctx, const ast::ClassDef &tree);
    void postTransformClassDef(core::Context ctx, const ast::ClassDef &tree);
    void preTransformMethodDef(core::Context ctx, const ast::MethodDef &tree);
    void postTransformRuntimeMethodDefinition(core::Context ctx, const ast::RuntimeMethodDefinition &tree);
    void preTransformSend(core::Context ctx, const ast::Send &tree);

    // Finds the `sig` (if any) belonging to the method at queryLoc, and parses it, reporting any
    // type errors found in the sig. Use this when the caller needs the parsed type information
    // (e.g., a method's argument types).
    static std::optional<Result> findSignature(core::Context ctx, const ast::ExpressionPtr &tree, core::Loc queryLoc);
    static std::optional<Result> findSignature(core::Context ctx, const ast::ClassDef &tree, core::Loc queryLoc);

    // Finds the `sig` (if any) belonging to the method at queryLoc, without parsing it. Use this
    // when the caller only needs syntactic information about the `sig` (e.g., its location, or
    // whether it has a block), so that we avoid the redundant, error-reporting parse entirely.
    static const ast::Send *findSignatureSend(core::Context ctx, const ast::ExpressionPtr &tree, core::Loc queryLoc);
    static const ast::Send *findSignatureSend(core::Context ctx, const ast::ClassDef &tree, core::Loc queryLoc);
};

} // namespace sorbet::sig_finder

#endif // SORBET_SIG_FINDER
