#include "main/pipeline/DefinitionLinesDenylistEnforcer.h"
#include "ast/treemap/treemap.h"

namespace sorbet::pipeline::definition_checker {
namespace {
bool shouldSkip(core::GlobalState &gs, core::SymbolRef sym, UnorderedSet<core::NameRef> &ignoredSymbols) {
    auto name = sym.name(gs);
    if (ignoredSymbols.contains(name)) {
        return true;
    }

    if (name.kind() == core::NameKind::UNIQUE) {
        auto data = name.dataUnique(gs);
        if (ignoredSymbols.contains(data->original)) {
            return true;
        }
    }
    return false;
}
} // namespace

void checkNoDefinitionsInsideProhibitedLines(core::GlobalState &gs, UnorderedSet<core::FileRef> &frefs) {
    std::vector<std::pair<core::SymbolRef::Kind, uint32_t>> symbolTypes = {
        {core::SymbolRef::Kind::ClassOrModule, gs.classAndModulesUsed()},
        {core::SymbolRef::Kind::Method, gs.methodsUsed()},
        {core::SymbolRef::Kind::FieldOrStaticField, gs.fieldsUsed()},
        {core::SymbolRef::Kind::TypeArgument, gs.typeArgumentsUsed()},
        // {core::SymbolRef::Kind::TypeMember, gs.typeMembersUsed()},
    };

    UnorderedSet<core::NameRef> ignoredSymbols = {
        core::Names::requiredAncestors(),
        core::Names::requiredAncestorsLin(),
        core::Names::staticInit()
    };

    auto everFailed = false;
    for (auto [kind, used] : symbolTypes) {
        for (uint32_t idx = 1; idx < used; idx++) {
            auto sym = core::SymbolRef(gs, kind, idx);

            if (shouldSkip(gs, sym, ignoredSymbols)) {
                continue;
            }

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
