#include "main/pipeline/definition_checker/DefinitionLinesDenylistEnforcer.h"
#include "ast/treemap/treemap.h"

namespace sorbet::pipeline::definition_checker {

void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, core::FileRef fref, int prohibitedLinesStart,
                                             int prohibitedLinesEnd) {
    std::vector<std::pair<core::SymbolRef::Kind, uint32_t>> symbolTypes = {
        {core::SymbolRef::Kind::ClassOrModule, gs.classAndModulesUsed()},
        {core::SymbolRef::Kind::Method, gs.methodsUsed()},
        {core::SymbolRef::Kind::FieldOrStaticField, gs.fieldsUsed()},
        {core::SymbolRef::Kind::TypeArgument, gs.typeArgumentsUsed()},
        {core::SymbolRef::Kind::TypeMember, gs.typeMembersUsed()},
    };

    for (auto [kind, used] : symbolTypes) {
        for (uint32_t idx = 1; idx < used; idx++) {
            auto sym = core::SymbolRef(gs, kind, idx);
            if (!sym.exists()) {
                continue;
            }

            auto locs = sym.locs(gs);
            for (auto loc : locs) {
                if (loc.file() != fref) {
                    continue;
                }
                auto [detailStart, detailEnd] = loc.position(gs);
                ENFORCE(!(detailStart.line >= prohibitedLinesStart && detailEnd.line <= prohibitedLinesEnd));
            }
        }
    }
}
} // namespace sorbet::pipeline::definition_checker
