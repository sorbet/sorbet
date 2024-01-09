#ifndef SORBET_PIPELINE_DEFINITION_CHECKER
#define SORBET_PIPELINE_DEFINITION_CHECKER

#include "ast/Trees.h"
#include "common/common.h"
#include "core/Context.h"
#include "core/FileRef.h"

namespace sorbet::pipeline::definition_checker {
void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, UnorderedSet<core::FileRef> &frefs);

} // namespace sorbet::pipeline::definition_checker
#endif
