#include "core/insert_method/insert_method.h"

namespace sorbet::core {

vector<core::AutocorrectSuggestion::Edit> insert_method::run(core::Loc classOrModuleDeclaredAt,
                                                             core::Loc classOrModuleEndsAt) {
    auto hasSingleLineDefinition =
        classOrModuleDeclaredAt.toDetails(ctx).first.line == classOrModuleEndsAt.toDetails(ctx).second.line;

    auto [endLoc, indentLength] = classOrModuleEndsAt.findStartOfIndentation(ctx);
    string classOrModuleIndent(indentLength, ' ');
    auto editLoc = endLoc.adjust(ctx, -indentLength, 0);

    vector<core::AutocorrectSuggestion::Edit> edits;
    if (hasSingleLineDefinition) {
        auto endRange = classOrModuleEndsAt.adjust(ctx, -3, 0);
        if (endRange.source(ctx) != "end") {
            return edits;
        }
        auto withSemi = endRange.adjust(ctx, -2 /* "; " */, -3 /* "end" */);
        if (withSemi.source(ctx) == "; ") {
            endRange = withSemi.join(endRange);
        }

        // Then, modify our insertion strategy such that we add new methods to the top of the class/module
        // body rather than the bottom. This is a trick to ensure that we put the new methods within the new
        // class/module body that we just created.
        editLoc = endRange;
    }

    fmt::memory_buffer buf;

    auto idx = -1;
    for (auto proto : missingAbstractMethods) {
        idx++;
        errorBuilder.addErrorLine(proto.data(ctx)->loc(), "`{}` defined here", proto.data(ctx)->name.show(ctx));

        auto indentedMethodDefinition = defineInheritedAbstractMethod(ctx, sym, proto, classOrModuleIndent);
        if (hasSingleLineDefinition) {
            fmt::format_to(back_inserter(buf), "\n{}", indentedMethodDefinition);
        } else if (idx + 1 < missingAbstractMethods.size()) {
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
