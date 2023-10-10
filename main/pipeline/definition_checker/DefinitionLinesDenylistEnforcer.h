#ifndef SORBET_PIPELINE_DEFINITION_CHECKER
#define SORBET_PIPELINE_DEFINITION_CHECKER

#include "ast/Trees.h"
#include "core/Context.h"
#include "core/FileRef.h"

namespace sorbet::pipeline::definition_checker {
void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, core::FileRef fref, int prohibitedLinesStart,
                                             int prohibitedLinesEnd);

} // namespace sorbet::pipeline::definition_checker
#endif
