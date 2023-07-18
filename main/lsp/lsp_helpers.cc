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

// iff a sig has more than this many parameters, then print it as a multi-line sig.
constexpr int MAX_PRETTY_SIG_ARGS = 4;
// iff a `def` would be this wide or wider, expand it to be a multi-line def.
constexpr int MAX_PRETTY_WIDTH = 80;

string prettySigForMethod(const core::GlobalState &gs, core::MethodRef method, const core::TypePtr &receiver,
                          core::TypePtr retType, const core::TypeConstraint *constraint) {
    ENFORCE(method.exists());
    ENFORCE(method.data(gs)->dealiasMethod(gs) == method);
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

    if (!retType) {
        retType = getResultType(gs, method.data(gs)->resultType, method, receiver, constraint);
    }
    string methodReturnType =
        (retType == core::Types::void_()) ? "void" : absl::StrCat("returns(", retType.show(gs), ")");
    vector<string> typeAndArgNames;

    vector<string> flags;
    auto sym = method.data(gs);
    string sigCall = "sig";
    if (sym->flags.isFinal) {
        sigCall = "sig(:final)";
    }
    if (sym->flags.isAbstract) {
        flags.emplace_back("abstract");
    }
    if (sym->flags.isOverridable) {
        flags.emplace_back("overridable");
    }
    if (sym->flags.isOverride) {
        flags.emplace_back("override");
    }
    for (auto &argSym : method.data(gs)->arguments) {
        // Don't display synthetic arguments (like blk).
        if (!argSym.isSyntheticBlockArgument()) {
            typeAndArgNames.emplace_back(absl::StrCat(
                argSym.argumentName(gs), ": ", getResultType(gs, argSym.type, method, receiver, constraint).show(gs)));
        }
    }

    string flagString = "";
    if (!flags.empty()) {
        flagString = fmt::format("{}.", fmt::join(flags, "."));
    }
    string paramsString = "";
    if (!typeAndArgNames.empty()) {
        paramsString = fmt::format("params({}).", fmt::join(typeAndArgNames, ", "));
    }

    auto oneline = fmt::format("{} {{{}{}{}}}", sigCall, flagString, paramsString, methodReturnType);
    if (oneline.size() <= MAX_PRETTY_WIDTH && typeAndArgNames.size() <= MAX_PRETTY_SIG_ARGS) {
        return oneline;
    }

    if (!flags.empty()) {
        flagString = fmt::format("{}\n  .", fmt::join(flags, "\n  ."));
    }
    if (!typeAndArgNames.empty()) {
        paramsString = fmt::format("params(\n    {}\n  )\n  .", fmt::join(typeAndArgNames, ",\n    "));
    }
    return fmt::format("{} do\n  {}{}{}\nend", sigCall, flagString, paramsString, methodReturnType);
}

string prettyDefForMethod(const core::GlobalState &gs, core::MethodRef method) {
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }
    auto methodData = method.data(gs);

    string visibility = "";
    if (methodData->flags.isPrivate) {
        visibility = "private ";
    } else if (methodData->flags.isProtected) {
        visibility = "protected ";
    }

    auto methodNameRef = methodData->name;
    ENFORCE(methodNameRef.exists());
    string methodName = "???";
    if (methodNameRef.exists()) {
        methodName = methodNameRef.toString(gs);
    }
    string methodNamePrefix = "";
    if (methodData->owner.exists() && methodData->owner.data(gs)->attachedClass(gs).exists()) {
        methodNamePrefix = "self.";
    }
    vector<string> prettyArgs;
    const auto &arguments = methodData->dealiasMethod(gs).data(gs)->arguments;
    ENFORCE(!arguments.empty(), "Should have at least a block arg");
    for (const auto &argSym : arguments) {
        // Don't display synthetic arguments (like blk).
        if (argSym.isSyntheticBlockArgument()) {
            continue;
        }
        string prefix = "";
        string suffix = "";
        if (argSym.flags.isRepeated) {
            if (argSym.flags.isKeyword) {
                prefix = "**"; // variadic keyword args
            } else {
                prefix = "*"; // rest args
            }
        } else if (argSym.flags.isKeyword) {
            if (argSym.flags.isDefault) {
                suffix = ": …"; // optional keyword (has a default value)
            } else {
                suffix = ":"; // required keyword
            }
        } else if (argSym.flags.isBlock) {
            prefix = "&";
        } else if (argSym.flags.isDefault) {
            suffix = "=…";
        }
        prettyArgs.emplace_back(fmt::format("{}{}{}", prefix, argSym.argumentName(gs), suffix));
    }

    string argListPrefix = "";
    string argListSeparator = "";
    string argListSuffix = "";
    if (prettyArgs.size() > 0) {
        argListPrefix = "(";
        argListSeparator = ", ";
        argListSuffix = ")";
    }

    auto result = fmt::format("{}def {}{}{}{}{}; end", visibility, methodNamePrefix, methodName, argListPrefix,
                              fmt::join(prettyArgs, argListSeparator), argListSuffix);
    if (prettyArgs.size() > 0 && result.length() >= MAX_PRETTY_WIDTH) {
        argListPrefix = "(\n  ";
        argListSeparator = ",\n  ";
        argListSuffix = "\n)";
        result = fmt::format("{}def {}{}{}{}{}\nend", visibility, methodNamePrefix, methodName, argListPrefix,
                             fmt::join(prettyArgs, argListSeparator), argListSuffix);
    }
    return result;
}

string prettyTypeForMethod(const core::GlobalState &gs, core::MethodRef method, const core::TypePtr &receiver,
                           const core::TypePtr &retType, const core::TypeConstraint *constraint) {
    return fmt::format("{}\n{}",
                       prettySigForMethod(gs, method.data(gs)->dealiasMethod(gs), receiver, retType, constraint),
                       prettyDefForMethod(gs, method));
}

string prettyTypeForConstant(const core::GlobalState &gs, core::SymbolRef constant) {
    // Request that the constant already be dealiased, rather than dealias here to avoid defensively dealiasing.
    // We should understand where dealias calls go.
    ENFORCE(constant == constant.dealias(gs));

    if (constant == core::Symbols::StubModule()) {
        return "This constant is not defined";
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

core::TypePtr getResultType(const core::GlobalState &gs, const core::TypePtr &type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const core::TypeConstraint *constr) {
    auto resultType = type;
    if (core::is_proxy_type(receiver)) {
        receiver = receiver.underlying(gs);
    }
    if (auto *applied = core::cast_type<core::AppliedType>(receiver)) {
        /* instantiate generic classes */
        resultType = core::Types::resultTypeAsSeenFrom(gs, resultType, inWhat.enclosingClass(gs), applied->klass,
                                                       applied->targs);
    }
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }
    if (receiver) {
        resultType = core::Types::replaceSelfType(gs, resultType, receiver); // instantiate self types
    }
    if (constr) {
        resultType = core::Types::instantiate(gs, resultType, *constr); // instantiate generic methods
    }
    return resultType;
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
