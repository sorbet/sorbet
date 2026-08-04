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

    static std::optional<Result> findSignature(core::Context ctx, const ast::ExpressionPtr &tree, core::Loc queryLoc);
    static std::optional<Result> findSignature(core::Context ctx, const ast::ClassDef &tree, core::Loc queryLoc);
};

} // namespace sorbet::sig_finder

#endif // SORBET_SIG_FINDER
