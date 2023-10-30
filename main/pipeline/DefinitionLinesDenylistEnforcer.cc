#include "main/pipeline/DefinitionLinesDenylistEnforcer.h"
#include "ast/treemap/treemap.h"

namespace sorbet::pipeline::definition_checker {

void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, UnorderedSet<core::FileRef> &frefs) {
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
            auto locs = sym.locs(gs);
            for (auto loc : locs) {
                if (!frefs.contains(loc.file())) {
                    continue;
                }
                auto midPoint = loc.file().data(gs).source().size() / 2;
                auto cond = loc.beginPos() >= midPoint && loc.endPos() >= midPoint;



                if (kind != core::SymbolRef::Kind::TypeMember) {
                    ENFORCE(cond, fmt::format("beginPos: {}, endPos: {}, midPoint: {}, kind: {}, name: {}\n", loc.beginPos(), loc.endPos(), midPoint, kind, sym.toStringFullName(gs)));
                } else {
                    if (!cond) {
                        fmt::print("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nType memeber failed to update location beginPos: {}, endPos: {}, midPoint: {}, kind: {}, name: {}\n\n", loc.beginPos(), loc.endPos(), midPoint, kind, sym.toStringFullName(gs));
                    }
                }
            }
        }
    }
}
} // namespace sorbet::pipeline::definition_checker
