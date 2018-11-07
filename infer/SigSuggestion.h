#ifndef SORBET_SIG_SUGGESTION_H
#define SORBET_SIG_SUGGESTION_H

#include "cfg/CFG.h"

namespace sorbet::infer {
class SigSuggestion final {
public:
    static bool maybeSuggestSig(core::Context ctx, core::ErrorBuilder &e, std::unique_ptr<cfg::CFG> &cfg,
                                const std::shared_ptr<core::Type> &methodReturnType, core::TypeConstraint &constr);
};
} // namespace sorbet::infer

#endif // SORBET_SIG_SUGGESTION_H
