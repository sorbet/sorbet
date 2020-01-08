#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

vector<unique_ptr<Location>>
LSPLoop::extractLocations(const core::GlobalState &gs,
                          const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                          vector<unique_ptr<Location>> locations) const {
    for (auto &q : queryResponses) {
        core::Loc loc = q->getLoc();
        if (loc.exists() && loc.file().exists()) {
            auto fileIsTyped = loc.file().data(gs).strictLevel >= core::StrictLevel::True;
            // If file is untyped, only support responses involving constants and definitions.
            if (fileIsTyped || q->isConstant() || q->isField() || q->isDefinition()) {
                addLocIfExists(gs, locations, loc);
            }
        }
    }
    // Dedupe locations
    fast_sort(locations,
              [](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool { return a->cmp(*b) < 0; });
    locations.resize(std::distance(locations.begin(),
                                   std::unique(locations.begin(), locations.end(),
                                               [](const unique_ptr<Location> &a,
                                                  const unique_ptr<Location> &b) -> bool { return a->cmp(*b) == 0; })));
    return locations;
}

bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    auto data = sym.data(gs);
    if (data->isClassOrModule() && data->attachedClass(gs).exists()) {
        return true;
    }
    if (data->isClassOrModule() && data->superClass() == core::Symbols::StubModule()) {
        return true;
    }
    // static-init for a class
    if (data->name == core::Names::staticInit() ||
        // <unresolved-ancestors> is a fake method created to ensure IDE takes slow path for class hierarchy changes
        data->name == core::Names::unresolvedAncestors() || data->name == core::Names::Constants::AttachedClass()) {
        return true;
    }
    // static-init for a file
    if (data->name.data(gs)->kind == core::NameKind::UNIQUE &&
        data->name.data(gs)->unique.original == core::Names::staticInit()) {
        return true;
    }
    // <block>
    if (data->name.data(gs)->kind == core::NameKind::UNIQUE &&
        data->name.data(gs)->unique.original == core::Names::blockTemp()) {
        return true;
    }
    return false;
}

bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, string_view pattern) {
    string_view view = name.data(gs)->shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != string_view::npos;
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

string prettySigForMethod(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver,
                          core::TypePtr retType, const core::TypeConstraint *constraint) {
    ENFORCE(method.exists());
    ENFORCE(method.data(gs)->dealias(gs) == method);
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

    if (!retType) {
        retType = getResultType(gs, method.data(gs)->resultType, method, receiver, constraint);
    }
    string methodReturnType =
        (retType == core::Types::void_()) ? "void" : absl::StrCat("returns(", retType->show(gs), ")");
    vector<string> typeAndArgNames;

    vector<string> flags;
    const core::SymbolData &sym = method.data(gs);
    string accessFlagString = "";
    string sigCall = "sig";
    if (sym->isMethod()) {
        if (sym->isFinalMethod()) {
            sigCall = "sig(:final)";
        }
        if (sym->isAbstract()) {
            flags.emplace_back("abstract");
        }
        if (sym->isOverridable()) {
            flags.emplace_back("overridable");
        }
        if (sym->isOverride()) {
            flags.emplace_back("override");
        }
        if (sym->isPrivate()) {
            accessFlagString = "private ";
        }
        if (sym->isProtected()) {
            accessFlagString = "protected ";
        }
        for (auto &argSym : method.data(gs)->arguments()) {
            // Don't display synthetic arguments (like blk).
            if (!argSym.isSyntheticBlockArgument()) {
                typeAndArgNames.emplace_back(
                    absl::StrCat(argSym.argumentName(gs), ": ",
                                 getResultType(gs, argSym.type, method, receiver, constraint)->show(gs)));
            }
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

    auto oneline =
        fmt::format("{}{} {{{}{}{}}}", accessFlagString, sigCall, flagString, paramsString, methodReturnType);
    if (oneline.size() <= MAX_PRETTY_WIDTH && typeAndArgNames.size() <= MAX_PRETTY_SIG_ARGS) {
        return oneline;
    }

    if (!flags.empty()) {
        flagString = fmt::format("{}\n  .", fmt::join(flags, "\n  ."));
    }
    if (!typeAndArgNames.empty()) {
        paramsString = fmt::format("params(\n    {}\n  )\n  .", fmt::join(typeAndArgNames, ",\n    "));
    }
    return fmt::format("{}{} do\n  {}{}{}\nend", accessFlagString, sigCall, flagString, paramsString, methodReturnType);
}

string prettyDefForMethod(const core::GlobalState &gs, core::SymbolRef method) {
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }
    auto methodData = method.data(gs);

    auto methodNameRef = methodData->name;
    ENFORCE(methodNameRef.exists());
    string methodName = "???";
    if (methodNameRef.exists()) {
        methodName = methodNameRef.data(gs)->toString(gs);
    }
    string methodNamePrefix = "";
    if (methodData->owner.exists() && methodData->owner.data(gs)->isClassOrModule() &&
        methodData->owner.data(gs)->attachedClass(gs).exists()) {
        methodNamePrefix = "self.";
    }
    vector<string> prettyArgs;
    const auto &arguments = methodData->dealias(gs).data(gs)->arguments();
    ENFORCE(!arguments.empty(), "Should have at least a block arg");
    for (const auto &argSym : arguments) {
        // Don't display synthetic arguments (like blk).
        if (argSym.isSyntheticBlockArgument()) {
            continue;
        }
        string prefix = "";
        string suffix = "";
        if (argSym.flags.isKeyword) {
            if (argSym.flags.isRepeated) {
                prefix = "**"; // variadic keyword args
            } else if (argSym.flags.isDefault) {
                suffix = ": …"; // optional keyword (has a default value)
            } else {
                suffix = ":"; // required keyword
            }
        } else if (argSym.flags.isRepeated) {
            prefix = "*";
        } else if (argSym.flags.isBlock) {
            prefix = "&";
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

    auto result = fmt::format("def {}{}{}{}{}; end", methodNamePrefix, methodName, argListPrefix,
                              fmt::join(prettyArgs, argListSeparator), argListSuffix);
    if (prettyArgs.size() > 0 && result.length() >= MAX_PRETTY_WIDTH) {
        argListPrefix = "(\n  ";
        argListSeparator = ",\n  ";
        argListSuffix = "\n)";
        result = fmt::format("def {}{}{}{}{}\nend", methodNamePrefix, methodName, argListPrefix,
                             fmt::join(prettyArgs, argListSeparator), argListSuffix);
    }
    return result;
}

string prettyTypeForMethod(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver,
                           core::TypePtr retType, const core::TypeConstraint *constraint) {
    return fmt::format("{}\n{}", prettySigForMethod(gs, method.data(gs)->dealias(gs), receiver, retType, constraint),
                       prettyDefForMethod(gs, method));
}

string prettyTypeForConstant(const core::GlobalState &gs, core::SymbolRef constant) {
    // Request that the constant already be dealiased, rather than dealias here to avoid defensively dealiasing.
    // We should understand where dealias calls go.
    ENFORCE(constant == constant.data(gs)->dealias(gs));

    core::TypePtr result;
    if (constant.data(gs)->isClassOrModule()) {
        auto targetClass = constant;
        if (!targetClass.data(gs)->attachedClass(gs).exists()) {
            targetClass = targetClass.data(gs)->lookupSingletonClass(gs);
        }
        result = targetClass.data(gs)->externalType(gs);
    } else {
        auto resultType = constant.data(gs)->resultType;
        result = resultType == nullptr ? core::Types::untyped(gs, constant) : resultType;
    }

    if (constant.data(gs)->isTypeAlias()) {
        // By wrapping the type in `MetaType`, it displays as `<Type: Foo>` rather than `Foo`.
        result = core::make_type<core::MetaType>(result);
    }
    return result->showWithMoreInfo(gs);
}

core::TypePtr getResultType(const core::GlobalState &gs, core::TypePtr type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const core::TypeConstraint *constr) {
    core::Context ctx(gs, inWhat);
    auto resultType = type;
    if (auto *proxy = core::cast_type<core::ProxyType>(receiver.get())) {
        receiver = proxy->underlying();
    }
    if (auto *applied = core::cast_type<core::AppliedType>(receiver.get())) {
        /* instantiate generic classes */
        resultType = core::Types::resultTypeAsSeenFrom(ctx, resultType, inWhat.data(ctx)->enclosingClass(ctx),
                                                       applied->klass, applied->targs);
    }
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }
    if (receiver) {
        resultType = core::Types::replaceSelfType(ctx, resultType, receiver); // instantiate self types
    }
    if (constr) {
        resultType = core::Types::instantiate(ctx, resultType, *constr); // instantiate generic methods
    }
    return resultType;
}

SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef symbol) {
    auto sym = symbol.data(gs);
    if (sym->isClassOrModule()) {
        if (sym->isClassOrModuleModule()) {
            return SymbolKind::Module;
        }
        if (sym->isClassOrModuleClass()) {
            return SymbolKind::Class;
        }
    } else if (sym->isMethod()) {
        if (sym->name == core::Names::initialize()) {
            return SymbolKind::Constructor;
        } else {
            return SymbolKind::Method;
        }
    } else if (sym->isField()) {
        return SymbolKind::Field;
    } else if (sym->isStaticField()) {
        return SymbolKind::Constant;
    } else if (sym->isTypeMember()) {
        return SymbolKind::TypeParameter;
    } else if (sym->isTypeArgument()) {
        return SymbolKind::TypeParameter;
    }
    return SymbolKind::Unknown;
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
            // ASSUMPTION: We either hit the start of file, a `sig do` or an `end`
            it++;
            while (
                // SOF
                it != all_lines.rend()
                // Start of sig block
                && !absl::StartsWith(absl::StripAsciiWhitespace(*it), "sig do")
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
            ENFORCE(absl::StartsWith(line, "sig do"));

            // Stop looking if this is a single-line block e.g `sig do; <block>; end`
            if (absl::StartsWith(line, "sig do;") && absl::EndsWith(line, "end")) {
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

            documentation_lines.push_back(comment);

            // Account for yarddoc lines by inserting an extra newline right before
            // the yarddoc line (note that we are reverse iterating)
            if (absl::StartsWith(comment, "@")) {
                documentation_lines.push_back(string_view(""));
            }
        }

        // No other cases applied to this line, so stop looking.
        else {
            break;
        }
    }

    string documentation = absl::StrJoin(documentation_lines.rbegin(), documentation_lines.rend(), "\n");
    documentation = absl::StripTrailingAsciiWhitespace(documentation);

    if (documentation.empty())
        return nullopt;
    else {
        return documentation;
    }
}
} // namespace sorbet::realmain::lsp
