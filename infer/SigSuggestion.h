#ifndef SORBET_SIG_SUGGESTION_H
#define SORBET_SIG_SUGGESTION_H

#include "cfg/CFG.h"

namespace sorbet::infer {
class SigSuggestion final {
public:
    static std::optional<core::AutocorrectSuggestion> maybeSuggestSig(core::Context ctx, std::unique_ptr<cfg::CFG> &cfg,
                                                                      const core::TypePtr &methodReturnType,
                                                                      core::TypeConstraint &constr);
};
} // namespace sorbet::infer

#endif // SORBET_SIG_SUGGESTION_H
