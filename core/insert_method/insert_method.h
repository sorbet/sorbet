#ifndef SORBET_CORE_INSERT_METHOD
#define SORBET_CORE_INSERT_METHOD

#include "core/AutocorrectSuggestion.h"

namespace sorbet::core {

class insert_method {
public:
    static std::vector<core::AutocorrectSuggestion::Edit> run(core::Loc classOrModuleDeclaredAt,
                                                              core::Loc classOrModuleEndsAt);
};

} // namespace sorbet::core

#endif // SORBET_CORE_INSERT_METHOD
