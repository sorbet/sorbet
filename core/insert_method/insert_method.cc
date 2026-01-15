#include "core/insert_method/insert_method.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "core/source_generator/source_generator.h"

using namespace std;

namespace sorbet::core {

namespace {
string defineInheritedAbstractMethod(const core::GlobalState &gs, const core::ClassOrModuleRef sym,
                                     const core::MethodRef abstractMethodRef, const string &classOrModuleIndent) {
    auto showOptions = core::ShowOptions().withUseValidSyntax().withConcretizeIfAbstract();
    if (sym.data(gs)->attachedClass(gs).exists()) {
        showOptions = showOptions.withForceSelfPrefix();
    }
    auto methodDefinition = core::source_generator::prettyTypeForMethod(gs, abstractMethodRef, nullptr, showOptions);

    vector<string> indentedLines;
    absl::c_transform(
        absl::StrSplit(methodDefinition, "\n"), std::back_inserter(indentedLines),
        [classOrModuleIndent](auto &line) -> string { return fmt::format("{}  {}", classOrModuleIndent, line); });
    auto indentedMethodDefinition = absl::StrJoin(indentedLines, "\n");
    return indentedMethodDefinition;
}
} // namespace

vector<core::AutocorrectSuggestion::Edit>
insert_method::run(const core::GlobalState &gs, absl::Span<const core::MethodRef> toInsert,
                   core::ClassOrModuleRef inWhere, core::Loc classOrModuleDeclaredAt, core::Loc classOrModuleEndsAt) {
    auto hasSingleLineDefinition =
        classOrModuleDeclaredAt.toDetails(gs).first.line == classOrModuleEndsAt.toDetails(gs).second.line;

    auto [endLoc, indentLength] = classOrModuleEndsAt.findStartOfIndentation(gs);
    string classOrModuleIndent(indentLength, ' ');
    auto editLoc = endLoc.adjust(gs, -indentLength, 0);

    vector<core::AutocorrectSuggestion::Edit> edits;
    if (hasSingleLineDefinition) {
        auto endRange = classOrModuleEndsAt.adjust(gs, -3, 0);
        if (endRange.source(gs) != "end") {
            return edits;
        }
        auto withSemi = endRange.adjust(gs, -2 /* "; " */, -3 /* "end" */);
        if (withSemi.source(gs) == "; ") {
            endRange = withSemi.join(endRange);
        }

        // Then, modify our insertion strategy such that we add new methods to the top of the class/module
        // body rather than the bottom. This is a trick to ensure that we put the new methods within the new
        // class/module body that we just created.
        editLoc = endRange;
    }

    fmt::memory_buffer buf;

    auto idx = -1;
    for (auto proto : toInsert) {
        idx++;

        auto indentedMethodDefinition = defineInheritedAbstractMethod(gs, inWhere, proto, classOrModuleIndent);
        if (hasSingleLineDefinition) {
            fmt::format_to(back_inserter(buf), "\n{}", indentedMethodDefinition);
        } else if (idx + 1 < toInsert.size()) {
            fmt::format_to(back_inserter(buf), "{}\n", indentedMethodDefinition);
        } else {
            fmt::format_to(back_inserter(buf), "{}\n{}", indentedMethodDefinition, classOrModuleIndent);
        }
    }

    if (hasSingleLineDefinition) {
        fmt::format_to(back_inserter(buf), "\n{}end", classOrModuleIndent);
    }

    auto editStr = to_string(buf);
    if (!editStr.empty()) {
        edits.emplace_back(core::AutocorrectSuggestion::Edit{editLoc, editStr});
    }

    return edits;
}

} // namespace sorbet::core
