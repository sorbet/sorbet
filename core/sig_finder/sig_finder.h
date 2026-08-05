#ifndef SORBET_SIG_FINDER
#define SORBET_SIG_FINDER

#include "resolver/type_syntax/type_syntax.h"

namespace sorbet::sig_finder {

class SigFinder {
public:
    struct Result {
        resolver::ParsedSig sig;
        const ast::Send &origSend;
        Result(resolver::ParsedSig &&sig, const ast::Send &origSend) : sig(std::move(sig)), origSend(origSend) {}
    };

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
