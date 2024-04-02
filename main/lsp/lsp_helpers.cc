#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/sort/sort.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    if (!sym.owner(gs).exists()) {
        return true;
    }
    if (sym.isClassOrModule() && sym.asClassOrModuleRef().data(gs)->attachedClass(gs).exists()) {
        return true;
    }
    if (sym.isClassOrModule() && sym.asClassOrModuleRef().data(gs)->superClass() == core::Symbols::StubModule()) {
        return true;
    }
    auto name = sym.name(gs);
    // static-init for a class
    if (name == core::Names::staticInit() ||
        // <unresolved-ancestors> is a fake method created to ensure IDE takes slow path for class hierarchy changes
        name == core::Names::unresolvedAncestors() || name == core::Names::Constants::AttachedClass()) {
        return true;
    }
    // TODO(jez) We probably want to get rid of anything with angle brackets (like what
    // completion.cc does) but that can be in another change.
    if (name == core::Names::beforeAngles()) {
        return true;
    }
    // static-init for a file
    if (name.kind() == core::NameKind::UNIQUE && name.dataUnique(gs)->original == core::Names::staticInit()) {
        return true;
    }
    // <block>
    if (name.kind() == core::NameKind::UNIQUE && name.dataUnique(gs)->original == core::Names::blockTemp()) {
        return true;
    }
    return false;
}

unique_ptr<MarkupContent> formatRubyMarkup(MarkupKind markupKind, string_view rubyMarkup,
                                           optional<string_view> explanation) {
    // format rubyMarkup
    string formattedTypeString;
    if (markupKind == MarkupKind::Markdown && rubyMarkup.length() > 0) {
        formattedTypeString = fmt::format("```ruby\n{}\n```", rubyMarkup);
    } else {
        formattedTypeString = string(rubyMarkup);
    }

    string content =
        absl::StrCat(formattedTypeString, explanation.has_value() ? "\n\n---\n\n" : "", explanation.value_or(""));

    return make_unique<MarkupContent>(markupKind, move(content));
}

string prettyTypeForConstant(const core::GlobalState &gs, core::SymbolRef constant) {
    if (constant == core::Symbols::StubModule()) {
        return "(unable to resolve constant)";
    }

    if (constant.isClassAlias(gs)) {
        auto dealiased = constant.dealias(gs);
        auto dealiasedShow =
            dealiased == core::Symbols::StubModule() ? "(unable to resolve constant)" : dealiased.show(gs);
        return fmt::format("{} = {}", constant.name(gs).show(gs), dealiasedShow);
    }

    core::TypePtr result;
    if (constant.isClassOrModule()) {
        auto targetClass = constant.asClassOrModuleRef();
        if (!targetClass.data(gs)->attachedClass(gs).exists()) {
            targetClass = targetClass.data(gs)->lookupSingletonClass(gs);
        }
        result = targetClass.data(gs)->externalType();
    } else {
        auto resultType = constant.resultType(gs);
        result = resultType == nullptr ? core::Types::untyped(constant) : resultType;
    }

    if (constant.isTypeAlias(gs)) {
        return fmt::format("T.type_alias {{{}}}", result.show(gs));
    } else {
        return result.showWithMoreInfo(gs);
    }
}

SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef symbol, bool isAttrBestEffortUIOnly) {
    if (symbol.isClassOrModule()) {
        auto klass = symbol.asClassOrModuleRef();
        if (klass.data(gs)->isModule()) {
            return SymbolKind::Module;
        }
        if (klass.data(gs)->isClass()) {
            return SymbolKind::Class;
        }
    } else if (symbol.isMethod()) {
        auto method = symbol.asMethodRef();
        if (method.data(gs)->name == core::Names::initialize()) {
            return SymbolKind::Constructor;
        } else if (isAttrBestEffortUIOnly) {
            return SymbolKind::Property;
        } else {
            return SymbolKind::Method;
        }
    } else if (symbol.isField(gs)) {
        return SymbolKind::Field;
    } else if (symbol.isStaticField(gs)) {
        return SymbolKind::Constant;
    } else if (symbol.isTypeMember()) {
        return SymbolKind::TypeParameter;
    } else if (symbol.isTypeArgument()) {
        return SymbolKind::TypeParameter;
    }
    return SymbolKind::Unknown;
}

namespace {

// Checks if s is a subclass of root or contains root as a mixin, and updates visited and memoized vectors.
bool isSubclassOrMixin(const core::GlobalState &gs, core::ClassOrModuleRef root, core::ClassOrModuleRef s,
                       std::vector<bool> &memoized, std::vector<bool> &visited) {
    // don't visit the same class twice
    if (visited[s.id()] == true) {
        return memoized[s.id()];
    }
    visited[s.id()] = true;

    for (auto a : s.data(gs)->mixins()) {
        if (a == root) {
            memoized[s.id()] = true;
            return true;
        }
    }
    if (s.data(gs)->superClass().exists()) {
        memoized[s.id()] = isSubclassOrMixin(gs, root, s.data(gs)->superClass(), memoized, visited);
    }

    return memoized[s.id()];
}

} // namespace

// This is slow. See the comment in the header file.
vector<core::ClassOrModuleRef> getSubclassesSlow(const core::GlobalState &gs, core::ClassOrModuleRef sym,
                                                 bool includeSelf) {
    vector<bool> memoized(gs.classAndModulesUsed());
    vector<bool> visited(gs.classAndModulesUsed());
    memoized[sym.id()] = true;
    visited[sym.id()] = true;

    vector<core::ClassOrModuleRef> subclasses;
    for (uint32_t i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto s = core::ClassOrModuleRef(gs, i);
        if (!includeSelf && s == sym) {
            continue;
        }
        if (isSubclassOrMixin(gs, sym, s, memoized, visited)) {
            subclasses.emplace_back(s);
        }
    }
    return subclasses;
}

unique_ptr<core::lsp::QueryResponse>
skipLiteralIfMethodDef(vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) {
    for (auto &r : queryResponses) {
        if (r->isMethodDef()) {
            return move(r);
        } else if (!r->isLiteral()) {
            break;
        }
    }

    return move(queryResponses[0]);
}

unique_ptr<core::lsp::QueryResponse>
getQueryResponseForFindAllReferences(vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) {
    // Find all references might show an Ident last if its a `prop`, and the Ident will be the
    // synthetic local variable name of the method argument.
    auto firstResp = queryResponses[0]->isIdent();
    if (firstResp == nullptr) {
        return skipLiteralIfMethodDef(queryResponses);
    }

    for (auto resp = queryResponses.begin() + 1; resp != queryResponses.end(); ++resp) {
        // If this query response has the same location as the first ident response, keep skipping
        // up. Seeing a query response for the exact same loc suggests this was synthesized in
        // rewriter.
        if ((*resp)->getLoc() == firstResp->termLoc) {
            continue;
        }

        // It's always okay to skip literals for Find All References
        auto lit = (*resp)->isLiteral();
        if (lit != nullptr) {
            continue;
        }

        if ((*resp)->isMethodDef()) {
            return move(*resp);
        } else {
            return skipLiteralIfMethodDef(queryResponses);
        }
    }

    return skipLiteralIfMethodDef(queryResponses);
}

/**
 * Retrieves the documentation above a symbol.
 * - Returned documentation has one trailing newline (if it exists)
 * - Assumes that valid ruby syntax is used.
 * - Strips the first whitespace character from a comment e.g
 *      # a comment
 *      #a comment
 *   are the same.
 */
optional<string> findDocumentation(string_view sourceCode, int beginIndex) {
    // Everything in the file before the method definition.
    string_view preDefinition = sourceCode.substr(0, sourceCode.rfind('\n', beginIndex));

    // Get all the lines before it.
    std::vector<string_view> all_lines = absl::StrSplit(preDefinition, '\n');

    // if there are no lines before the method definition, we're at the top of the file.
    if (all_lines.empty()) {
        return nullopt;
    }

    std::vector<string_view> documentation_lines;

    // Iterate from the last line, to the first line
    for (auto it = all_lines.rbegin(); it != all_lines.rend(); it++) {
        string_view line = absl::StripAsciiWhitespace(*it);

        // Short circuit when line is empty
        if (line.empty()) {
            break;
        }

        // Handle single-line sig block
        else if (absl::StartsWith(line, "sig")) {
            // Do nothing for a one-line sig block
        }

        // Handle multi-line sig block
        else if (absl::StartsWith(line, "end")) {
            // ASSUMPTION: We either hit the start of file, a `sig do`/`sig(:final) do` or an `end`
            it++;
            while (
                // SOF
                it != all_lines.rend()
                // Start of sig block
                && !(absl::StartsWith(absl::StripAsciiWhitespace(*it), "sig do") ||
                     absl::StartsWith(absl::StripAsciiWhitespace(*it), "sig(:final) do"))
                // Invalid end keyword
                && !absl::StartsWith(absl::StripAsciiWhitespace(*it), "end")) {
                it++;
            };

            // We have either
            // 1) Reached the start of the file
            // 2) Found a `sig do`
            // 3) Found an invalid end keyword
            if (it == all_lines.rend() || absl::StartsWith(absl::StripAsciiWhitespace(*it), "end")) {
                break;
            }

            // Reached a sig block.
            line = absl::StripAsciiWhitespace(*it);
            ENFORCE(absl::StartsWith(line, "sig do") || absl::StartsWith(line, "sig(:final) do"));

            // Stop looking if this is a single-line block e.g `sig do; <block>; end`
            if ((absl::StartsWith(line, "sig do;") || absl::StartsWith(line, "sig(:final) do;")) &&
                absl::EndsWith(line, "end")) {
                break;
            }

            // Else, this is a valid sig block. Move on to any possible documentation.
        }

        // Handle a comment line. Do not count typing declarations.
        else if (absl::StartsWith(line, "#") && !absl::StartsWith(line, "# typed:")) {
            // Account for whitespace before comment e.g
            // # abc -> "abc"
            // #abc -> "abc"
            int skip_after_hash = absl::StartsWith(line, "# ") ? 2 : 1;

            string_view comment = line.substr(line.find('#') + skip_after_hash);

            documentation_lines.emplace_back(comment);

            // Account for yarddoc lines by inserting an extra newline right before
            // the yarddoc line (note that we are reverse iterating)
            if (absl::StartsWith(comment, "@")) {
                documentation_lines.emplace_back("");
            }
        }

        // No other cases applied to this line, so stop looking.
        else {
            break;
        }
    }

    string documentation = absl::StrJoin(documentation_lines.rbegin(), documentation_lines.rend(), "\n");
    string_view stripped = absl::StripTrailingAsciiWhitespace(documentation);
    if (stripped.size() != documentation.size()) {
        documentation.resize(stripped.size());
    }

    if (documentation.empty()) {
        return nullopt;
    } else {
        return documentation;
    }
}
} // namespace sorbet::realmain::lsp
