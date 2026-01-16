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
    // <unresolved-ancestors> is a fake method created to ensure IDE takes slow path for class hierarchy changes
    if (name == core::Names::unresolvedAncestors() || name == core::Names::Constants::AttachedClass()) {
        return true;
    }
    // TODO(jez) We probably want to get rid of anything with angle brackets (like what
    // completion.cc does) but that can be in another change.
    if (name == core::Names::beforeAngles()) {
        return true;
    }
    // internal representation of enums as classes
    if (sym.isClassOrModule() && sym.asClassOrModuleRef().data(gs)->name.isTEnumName(gs)) {
        return true;
    }
    // static-init for a class or file
    if (name.isAnyStaticInitName(gs)) {
        return true;
    }
    // <block>
    if (name.kind() == core::NameKind::UNIQUE && name.dataUnique(gs)->original == core::Names::blockTemp()) {
        return true;
    }

    if (name == core::Names::requiredAncestorsLin()) {
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
        const auto &resultType = constant.resultType(gs);
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
    } else if (symbol.isTypeParameter()) {
        return SymbolKind::TypeParameter;
    }
    return SymbolKind::Unknown;
}

namespace {

struct AccessorInfo {
    core::FieldRef fieldSymbol;
    core::MethodRef readerSymbol;
    core::MethodRef writerSymbol;
};

static const vector<core::NameRef> accessorNames = {
    core::Names::prop(),        core::Names::tokenProp(),    core::Names::timestampedTokenProp(),
    core::Names::createdProp(), core::Names::attrAccessor(),
};

static const vector<core::NameRef> writerNames = {
    core::Names::attrWriter(),
};

static const vector<core::NameRef> readerNames = {
    core::Names::const_(),
    core::Names::merchantProp(),
    core::Names::attrReader(),
};

bool definedByAccessorMethod(const core::GlobalState &gs, AccessorInfo &info) {
    auto method = info.readerSymbol.exists() ? info.readerSymbol : info.writerSymbol;
    ENFORCE(method.exists());

    // Check definition site of method for `prop`, `const`, etc. The loc for the method should begin with
    // `def|prop|const|...`.
    auto methodSource = method.data(gs)->loc().source(gs);
    if (!methodSource.has_value()) {
        return false;
    }
    // Common case: ordinary `def`. Fast reject.
    if (absl::StartsWith(methodSource.value(), "def")) {
        return false;
    }

    if (absl::c_any_of(accessorNames, [&methodSource, &gs](auto name) -> bool {
            return absl::StartsWith(methodSource.value(), name.toString(gs));
        })) {
        return true;
    } else if (absl::c_any_of(writerNames, [&methodSource, &gs](auto name) -> bool {
                   return absl::StartsWith(methodSource.value(), name.toString(gs));
               })) {
        return true;
    } else if (absl::c_any_of(readerNames, [&methodSource, &gs](auto name) -> bool {
                   return absl::StartsWith(methodSource.value(), name.toString(gs));
               })) {
        return true;
    } else {
        return false;
    }
}

} // namespace

vector<unique_ptr<TextDocumentEdit>> getQuickfixEdits(const LSPConfiguration &config, const core::GlobalState &gs,
                                                      const vector<core::AutocorrectSuggestion::Edit> &edits) {
    UnorderedMap<core::FileRef, vector<unique_ptr<TextEdit>>> editsByFile;
    for (auto &edit : edits) {
        auto range = Range::fromLoc(gs, edit.loc);
        if (range != nullptr) {
            editsByFile[edit.loc.file()].emplace_back(make_unique<TextEdit>(move(range), edit.replacement));
        }
    }

    vector<unique_ptr<TextDocumentEdit>> documentEdits;
    for (auto &[file, edits] : editsByFile) {
        // TODO: Document version
        documentEdits.emplace_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject()), move(edits)));
    }
    return documentEdits;
}

void addOtherAccessorSymbols(const core::GlobalState &gs, core::SymbolRef symbol,
                             core::lsp::Query::Symbol::STORAGE &symbols) {
    AccessorInfo info;

    core::SymbolRef owner = symbol.owner(gs);
    if (!owner.exists() || !owner.isClassOrModule()) {
        return;
    }
    core::ClassOrModuleRef ownerCls = owner.asClassOrModuleRef();

    string_view baseName;

    string symbolName = symbol.name(gs).toString(gs);
    // Extract the base name from `symbol`.
    if (absl::StartsWith(symbolName, "@")) {
        if (!symbol.isField(gs)) {
            return;
        }
        info.fieldSymbol = symbol.asFieldRef();
        baseName = string_view(symbolName).substr(1);
    } else if (absl::EndsWith(symbolName, "=")) {
        if (!symbol.isMethod()) {
            return;
        }
        info.writerSymbol = symbol.asMethodRef();
        baseName = string_view(symbolName).substr(0, symbolName.length() - 1);
    } else {
        if (!symbol.isMethod()) {
            return;
        }
        info.readerSymbol = symbol.asMethodRef();
        baseName = symbolName;
    }

    // Find the other associated symbols.
    if (!info.fieldSymbol.exists()) {
        auto fieldNameStr = absl::StrCat("@", baseName);
        auto fieldName = gs.lookupNameUTF8(fieldNameStr);
        if (!fieldName.exists()) {
            // Field is not optional.
            return;
        }
        info.fieldSymbol = gs.lookupFieldSymbol(ownerCls, fieldName);
    }

    if (!info.readerSymbol.exists()) {
        auto readerName = gs.lookupNameUTF8(baseName);
        if (readerName.exists()) {
            info.readerSymbol = gs.lookupMethodSymbol(ownerCls, readerName);
        }
    }

    if (!info.writerSymbol.exists()) {
        auto writerNameStr = absl::StrCat(baseName, "=");
        auto writerName = gs.lookupNameUTF8(writerNameStr);
        if (writerName.exists()) {
            info.writerSymbol = gs.lookupMethodSymbol(ownerCls, writerName);
        }
    }

    // If this is an accessor, we should have a field and _at least_ one of reader or writer.
    if (!info.writerSymbol.exists() && !info.readerSymbol.exists()) {
        return;
    }

    // Use reader or writer to determine what type of field accessor we are dealing with (if any).
    if (definedByAccessorMethod(gs, info)) {
        if (info.fieldSymbol.exists() && symbol != info.fieldSymbol) {
            symbols.emplace_back(info.fieldSymbol);
        }
        if (info.readerSymbol.exists() && symbol != info.readerSymbol) {
            symbols.emplace_back(info.readerSymbol);
        }
        if (info.writerSymbol.exists() && symbol != info.writerSymbol) {
            symbols.emplace_back(info.writerSymbol);
        }
    }
}

namespace {

// Checks if s is a subclass of root or contains root as a mixin, and updates visited and memoized vectors.
bool isSubclassOrMixin(const core::GlobalState &gs, core::ClassOrModuleRef s, vector<bool> &memoized,
                       vector<bool> &visited) {
    // don't visit the same class twice
    if (visited[s.id()] == true) {
        return memoized[s.id()];
    }
    visited[s.id()] = true;

    for (auto a : s.data(gs)->mixins()) {
        if (memoized[a.id()]) {
            memoized[s.id()] = true;
            return true;
        }
    }
    if (s.data(gs)->superClass().exists()) {
        memoized[s.id()] = isSubclassOrMixin(gs, s.data(gs)->superClass(), memoized, visited);
    }

    return memoized[s.id()];
}

} // namespace

// This is slow. See the comment in the header file.
vector<core::ClassOrModuleRef> getSubclassesSlow(const core::GlobalState &gs, core::ClassOrModuleRef root,
                                                 bool includeRoot) {
    vector<bool> memoized(gs.classAndModulesUsed());
    vector<bool> visited(gs.classAndModulesUsed());
    memoized[root.id()] = true;
    visited[root.id()] = true;

    vector<core::ClassOrModuleRef> subclasses;
    for (uint32_t i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto s = core::ClassOrModuleRef(gs, i);
        if (!includeRoot && s == root) {
            continue;
        }
        if (isSubclassOrMixin(gs, s, memoized, visited)) {
            subclasses.emplace_back(s);
        }
    }
    return subclasses;
}

// This is slow. See the comment in the header file.
vector<core::ClassOrModuleRef> getSubclassesSlowMulti(const core::GlobalState &gs,
                                                      absl::Span<const core::ClassOrModuleRef> roots) {
    vector<bool> memoized(gs.classAndModulesUsed());
    vector<bool> visited(gs.classAndModulesUsed());
    for (auto root : roots) {
        memoized[root.id()] = true;
        visited[root.id()] = true;
    }

    vector<core::ClassOrModuleRef> subclasses;
    for (uint32_t i = 1; i < gs.classAndModulesUsed(); ++i) {
        auto s = core::ClassOrModuleRef(gs, i);
        if (isSubclassOrMixin(gs, s, memoized, visited)) {
            subclasses.emplace_back(s);
        }
    }
    return subclasses;
}

unique_ptr<core::lsp::QueryResponse>
skipLiteralIfPunnedKeywordArg(const core::GlobalState &gs,
                              vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) {
    auto &resp = queryResponses[0];
    auto *litResp = resp->isLiteral();
    if (litResp == nullptr || queryResponses.size() <= 1) {
        return nullptr;
    }
    auto identResp = queryResponses[1]->isIdent();
    if (identResp == nullptr || identResp->termLoc.adjust(gs, 0, -1) != litResp->termLoc) {
        return nullptr;
    }

    return move(queryResponses[1]);
}

unique_ptr<core::lsp::QueryResponse>
skipLiteralIfMethodDef(const core::GlobalState &gs, vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) {
    if (auto punnedKwarg = skipLiteralIfPunnedKeywordArg(gs, queryResponses)) {
        return punnedKwarg;
    }

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
getQueryResponseForFindAllReferences(const core::GlobalState &gs,
                                     vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) {
    // Find all references might show an Ident last if its a `prop`, and the Ident will be the
    // synthetic local variable name of the method argument.
    auto firstResp = queryResponses[0]->isIdent();
    if (firstResp == nullptr) {
        return skipLiteralIfMethodDef(gs, queryResponses);
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
            return skipLiteralIfMethodDef(gs, queryResponses);
        }
    }

    return skipLiteralIfMethodDef(gs, queryResponses);
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
    vector<string_view> all_lines = absl::StrSplit(preDefinition, '\n');

    // if there are no lines before the method definition, we're at the top of the file.
    if (all_lines.empty()) {
        return nullopt;
    }

    vector<string_view> documentation_lines;

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
        else if (absl::StartsWith(line, "}")) {
            // ASSUMPTION: We either hit the start of file or a `sig {`.
            //
            // Note that this will not properly handle brace blocks where the
            // closing brace is not on its own line, such as
            // ```
            // sig { params(foo: Foo)
            //       .returns(Bar) }
            // ```
            it++;
            line = absl::StripAsciiWhitespace(*it);
            while (
                // SOF
                it != all_lines.rend()
                // Start of sig block
                && !(absl::StartsWith(line, "sig {") || absl::StartsWith(line, "sig(:final) {"))
                // Invalid closing brace
                && !absl::StartsWith(line, "}")) {
                it++;
                if (it != all_lines.rend()) {
                    line = absl::StripAsciiWhitespace(*it);
                }
            };

            // We have either
            // 1) Reached the start of the file
            // 2) Found a `sig {`
            // 3) Found an invalid closing brace
            if (it == all_lines.rend() || absl::StartsWith(line, "}")) {
                break;
            }

            // Reached a sig block.
            ENFORCE(absl::StartsWith(line, "sig {") || absl::StartsWith(line, "sig(:final) {"));
        }

        // Handle multi-line sig block
        else if (absl::StartsWith(line, "end")) {
            // ASSUMPTION: We either hit the start of file, a `sig do`/`sig(:final) do` or an `end`
            it++;
            line = absl::StripAsciiWhitespace(*it);
            while (
                // SOF
                it != all_lines.rend()
                // Start of sig block
                && !(absl::StartsWith(line, "sig do") || absl::StartsWith(line, "sig(:final) do"))
                // Invalid end keyword
                && line != "end") {
                it++;
                if (it != all_lines.rend()) {
                    line = absl::StripAsciiWhitespace(*it);
                }
            };

            // We have either
            // 1) Reached the start of the file
            // 2) Found a `sig do`
            // 3) Found an invalid end keyword
            if (it == all_lines.rend() || line == "end") {
                break;
            }

            // Reached a sig block.
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
