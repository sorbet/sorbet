#include "main/pipeline/definition_checker/DefinitionLinesDenylistEnforcer.h"
#include "ast/treemap/treemap.h"

namespace sorbet::pipeline::definition_checker {
namespace {
void checkLoc(core::GlobalState &gs, const InlinedVector<core::Loc, 2> &locs, core::FileRef fref,
              int prohibitedLinesStart, int prohibitedLinesEnd) {
    for (auto loc : locs) {
        if (loc.file() != fref) {
            continue;
        }
        auto [detailStart, detailEnd] = loc.position(gs);
        ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
    }
}
} // namespace

void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, core::FileRef fref, int prohibitedLinesStart,
                                             int prohibitedLinesEnd) {
    for (int i = 0; i < gs.classAndModulesUsed(); i++) {
        auto classOrModule = core::ClassOrModuleRef(gs, i);
        auto locs = classOrModule.data(gs)->locs();
        checkLoc(gs, locs, fref, prohibitedLinesStart, prohibitedLinesEnd);
    }

    for (int i = 0; i < gs.methodsUsed(); i++) {
        auto method = core::MethodRef(gs, i);
        auto locs = method.data(gs)->locs();
        checkLoc(gs, locs, fref, prohibitedLinesStart, prohibitedLinesEnd);
    }

    for (int i = 0; i < gs.fieldsUsed(); i++) {
        auto field = core::FieldRef(gs, i);
        auto locs = field.data(gs)->locs();
        checkLoc(gs, locs, fref, prohibitedLinesStart, prohibitedLinesEnd);
    }

    for (int i = 0; i < gs.typeArgumentsUsed(); i++) {
        auto typeArgument = core::TypeArgumentRef(gs, i);
        auto locs = typeArgument.data(gs)->locs();
        checkLoc(gs, locs, fref, prohibitedLinesStart, prohibitedLinesEnd);
    }

    for (int i = 0; i < gs.typeMembersUsed(); i++) {
        auto typeMember = core::TypeMemberRef(gs, i);
        auto locs = typeMember.data(gs)->locs();
        checkLoc(gs, locs, fref, prohibitedLinesStart, prohibitedLinesEnd);
    }
}
} // namespace sorbet::pipeline::definition_checker
