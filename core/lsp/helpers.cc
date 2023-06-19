#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "core/core.h"

using namespace std;

namespace sorbet::core::lsp {

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

// iff a sig has more than this many parameters, then print it as a multi-line sig.
constexpr int MAX_PRETTY_SIG_ARGS = 4;
// iff a `def` would be this wide or wider, expand it to be a multi-line def.
constexpr int MAX_PRETTY_WIDTH = 80;

string prettySigForMethod(const core::GlobalState &gs, core::MethodRef method, const core::TypePtr &receiver,
                          core::TypePtr retType, const core::TypeConstraint *constraint,
                          const bool concretizeIfAbstract = false) {
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
    if (sym->flags.isAbstract && concretizeIfAbstract) {
        flags.emplace_back("override");
    } else if (sym->flags.isAbstract) {
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

string prettyDefForMethod(const core::GlobalState &gs, core::MethodRef method, const bool concretizeIfAbstract) {
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
    auto defaultArgumentPlaceholder = concretizeIfAbstract ? "T.let(T.unsafe(nil), T.untyped)" : "…";
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
                suffix = fmt::format(": {}", defaultArgumentPlaceholder); // optional keyword (has a default value)
            } else {
                suffix = ":"; // required keyword
            }
        } else if (argSym.flags.isBlock) {
            prefix = "&";
        } else if (argSym.flags.isDefault) {
            suffix = fmt::format("={}", defaultArgumentPlaceholder);
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
                           const core::TypePtr &retType, const core::TypeConstraint *constraint,
                           const bool concretizeIfAbstract = false) {
    return fmt::format(
        "{}\n{}",
        prettySigForMethod(gs, method.data(gs)->dealiasMethod(gs), receiver, retType, constraint, concretizeIfAbstract),
        prettyDefForMethod(gs, method, concretizeIfAbstract));
}

} // namespace sorbet::core::lsp
