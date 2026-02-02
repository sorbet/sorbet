#ifndef SORBET_CORE_INSERT_METHOD
#define SORBET_CORE_INSERT_METHOD

#include "core/AutocorrectSuggestion.h"
#include "core/GlobalState.h"

namespace sorbet::core {

class insert_method {
public:
    static std::vector<AutocorrectSuggestion::Edit> run(const GlobalState &gs, absl::Span<const MethodRef> toInsert,
                                                        ClassOrModuleRef inWhere, Loc classOrModuleDeclaredAt,
                                                        Loc classOrModuleEndsAt);
};

} // namespace sorbet::core

#endif // SORBET_CORE_INSERT_METHOD
