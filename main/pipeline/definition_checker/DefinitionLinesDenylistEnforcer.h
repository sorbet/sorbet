#ifndef SORBET_PIPELINE_DEFINITION_CHECKER
#define SORBET_PIPELINE_DEFINITION_CHECKER

#include "ast/Trees.h"
#include "core/Context.h"
#include "core/FileRef.h"

namespace sorbet::pipeline::definition_checker {
class DefinitionLinesDenylistEnforcer {
private:
    const core::FileRef file;
    const int prohibitedLinesStart;
    const int prohibitedLinesEnd;

    bool isAllowListed(core::Context ctx, core::SymbolRef sym);

    void checkLoc(core::Context ctx, core::Loc loc);

    void checkSym(core::Context ctx, core::SymbolRef sym);

public:
    DefinitionLinesDenylistEnforcer(core::FileRef file, int prohibitedLinesStart, int prohibitedLinesEnd)
        : file(file), prohibitedLinesStart(prohibitedLinesStart), prohibitedLinesEnd(prohibitedLinesEnd) {
        // Can be equal if file was empty.
        ENFORCE(prohibitedLinesStart <= prohibitedLinesEnd);
        ENFORCE(file.exists());
    };

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree);
    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree);
};

ast::ParsedFile checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, ast::ParsedFile what,
                                                        int prohibitedLinesStart, int prohibitedLinesEnd);
} // namespace sorbet::pipeline::definition_checker
#endif
