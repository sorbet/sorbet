#ifndef SORBET_CORE_SOURCE_GENERATOR_H
#define SORBET_CORE_SOURCE_GENERATOR_H

#include "core/ShowOptions.h"
#include "core/core.h"

namespace sorbet::core::source_generator {

core::TypePtr getResultType(const core::GlobalState &gs, const core::TypePtr &type, core::SymbolRef inWhat,
                            core::TypePtr receiver);

std::string prettyTypeForMethod(const core::GlobalState &gs, core::MethodRef method, const core::TypePtr &receiver,
                                const ShowOptions options);

} // namespace sorbet::core::source_generator
#endif // SORBET_CORE_SOURCE_GENERATOR_H
