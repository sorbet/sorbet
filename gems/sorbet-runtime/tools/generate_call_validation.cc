#include "common/common.h"

#include <cxxopts.hpp>
#include <variant>

using namespace std;

namespace sorbet::sorbet_runtime::generate_call_validation {

struct Options {
    bool bindCall;
};

enum class ValidatorKind {
    Method,
    MethodSkipReturn,
    Procedure,
};

enum class TypeKind {
    Simple,
    Complex,
};

string_view validatorKindToString(ValidatorKind kind) {
    switch (kind) {
        case ValidatorKind::Method:
            return "method"sv;
        case ValidatorKind::MethodSkipReturn:
            return "method_skip_return"sv;
        case ValidatorKind::Procedure:
            return "procedure"sv;
    }
}

string_view typeKindToString(TypeKind kind) {
    switch (kind) {
        case TypeKind::Simple:
            return "fast"sv;
        case TypeKind::Complex:
            return "medium"sv;
    }
}

constexpr size_t MAX_ARITY = 4;

variant<Options, int> parseArgs(int argc, char **argv) {
    cxxopts::Options options("generate_call_validation");
    options.add_options()("bind-call", "Whether to use UnboundMethod#bind_call or not", cxxopts::value<bool>());
    options.add_options()("h,help", "Show this help");

    try {
        auto raw = options.parse(argc, argv);
        if (raw["help"].as<bool>()) {
            fmt::print(stderr, "{}", options.help());
            return 0;
        }

        if (raw["bind-call"].count() == 0) {
            fmt::print(stderr, "Missing required option: --bind-call\n\n{}", options.help());
            return 1;
        }

        return Options{
            raw["bind-call"].as<bool>(),
        };
    } catch (cxxopts::OptionParseException &e) {
        fmt::print(stderr, "{}\n\n{}", e.what(), options.help());
        return 1;
    }
}

void generateCreateValidatorFastDispatcher(ValidatorKind kind, TypeKind type) {
    auto kindString = validatorKindToString(kind);
    auto typeString = typeKindToString(type);
    fmt::print("  def self.create_validator_{}_{}(mod, original_method, method_sig, original_visibility)\n", kindString,
               typeString);
    switch (kind) {
        case ValidatorKind::Method:
            fmt::print("    if method_sig.effective_return_type.is_a?(T::Private::Types::Void)\n"
                       "      raise 'Should have used create_validator_procedure_{}'\n"
                       "    end\n",
                       typeString);
            break;
        case ValidatorKind::MethodSkipReturn:
            break;
        case ValidatorKind::Procedure:
            break;
    }

    const char *rawTypeMethodCall;
    switch (type) {
        case TypeKind::Simple:
            rawTypeMethodCall = ".raw_type";
            break;
        case TypeKind::Complex:
            rawTypeMethodCall = "";
            break;
    }

    fmt::print("    # trampoline to reduce stack frame size\n");
    fmt::print("    arg_types = method_sig.arg_types\n");
    fmt::print("    case arg_types.length\n");
    for (size_t arity = 0; arity <= MAX_ARITY; arity++) {
        fmt::print("    when {}\n", arity);

        fmt::print("      create_validator_{}_{}{}(mod, original_method, method_sig, original_visibility", kindString,
                   typeString, arity);

        if (kind == ValidatorKind::Method) {
            fmt::print(", method_sig.effective_return_type{}", rawTypeMethodCall);
        }

        for (size_t i = 0; i < arity; i++) {
            fmt::print(",\n");
            fmt::print("                                    arg_types[{}][1]{}", i, rawTypeMethodCall);
        }
        fmt::print(")\n");
    }
    fmt::print("    else\n"
               "      raise 'should not happen'\n"
               "    end\n"
               "  end\n"
               "\n");
}

void generateCreateValidatorFast(const Options &options, ValidatorKind kind, TypeKind type, size_t arity) {
    const char *returnTypeArg;
    switch (kind) {
        case ValidatorKind::Method:
            returnTypeArg = ", return_type";
            break;
        case ValidatorKind::MethodSkipReturn:
        case ValidatorKind::Procedure:
            returnTypeArg = "";
            break;
    }
    fmt::print("  def self.create_validator_{}_{}{}(mod, original_method, method_sig, original_visibility{}",
               validatorKindToString(kind), typeKindToString(type), arity, returnTypeArg);
    for (size_t i = 0; i < arity; i++) {
        fmt::print(", arg{}_type", i);
    }
    fmt::print(")\n");

    fmt::print("    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |");
    for (size_t i = 0; i < arity; i++) {
        fmt::print("arg{}, ", i);
    }
    fmt::print("&blk|\n");

    fmt::print("      # This method is a manually sped-up version of more general code in `validate_call`\n");

    for (size_t i = 0; i < arity; i++) {
        switch (type) {
            case TypeKind::Simple:
                fmt::print("      unless arg{0}.is_a?(arg{0}_type)\n", i);
                break;
            case TypeKind::Complex:
                fmt::print("      unless arg{0}_type.valid?(arg{0})\n", i);
                break;
        }

        fmt::print("        CallValidation.report_error(\n"
                   "          method_sig,\n"
                   "          method_sig.arg_types[{0}][1].error_message_for_obj(arg{0}),\n"
                   "          'Parameter',\n"
                   "          method_sig.arg_types[{0}][0],\n"
                   "          arg{0}_type,\n"
                   "          arg{0},\n"
                   "          caller_offset: -1\n"
                   "        )\n"
                   "      end\n"
                   "\n",
                   i);
    }

    fmt::print("      # The following line breaks are intentional to show nice pry message\n");
    for (size_t i = 0; i < 10; i++) {
        fmt::print("\n");
    }
    fmt::print("      # PRY note:\n"
               "      # this code is sig validation code.\n"
               "      # Please issue `finish` to step out of it\n"
               "\n");

    const char *returnValueVar;
    switch (kind) {
        case ValidatorKind::Method:
            returnValueVar = "return_value = ";
            break;
        case ValidatorKind::MethodSkipReturn:
        case ValidatorKind::Procedure:
            returnValueVar = "";
            break;
    }
    fmt::print("      {}original_method", returnValueVar);
    if (options.bindCall) {
        fmt::print(".bind_call(self, ");
    } else {
        fmt::print(".bind(self).call(");
    }
    for (size_t i = 0; i < arity; i++) {
        fmt::print("arg{}, ", i);
    }
    fmt::print("&blk)\n");

    switch (kind) {
        case ValidatorKind::Procedure:
            fmt::print("      T::Private::Types::Void::VOID\n");
            break;

        case ValidatorKind::MethodSkipReturn:
            break;

        case ValidatorKind::Method: {
            const char *returnValueTypecheck;
            switch (type) {
                case TypeKind::Simple:
                    returnValueTypecheck = "return_value.is_a?(return_type)";
                    break;
                case TypeKind::Complex:
                    returnValueTypecheck = "return_type.valid?(return_value)";
                    break;
            }

            fmt::print("      unless {}\n"
                       "        message = method_sig.effective_return_type.error_message_for_obj(return_value)\n"
                       "        if message\n"
                       "          CallValidation.report_error(\n"
                       "            method_sig,\n"
                       "            message,\n"
                       "            'Return value',\n"
                       "            nil,\n"
                       "            method_sig.effective_return_type,\n"
                       "            return_value,\n"
                       "            caller_offset: -1\n"
                       "          )\n"
                       "        end\n"
                       "      end\n"
                       "      return_value\n",
                       returnValueTypecheck);
            break;
        }
    }

    fmt::print("    end\n"
               "  end\n"
               "\n");
}

// Emits one positional-arg check, matching the `unless argN_type.valid?(argN)`
// checks in the medium wrappers. `accessor` is how argN is read: "arg{i}" for a
// named block param, "args[{i}]" for the *args splat. `guard`, when non-empty, is
// a presence predicate OR-ed into the `unless` so the check is skipped when the
// arg was not passed (e.g. "args.length <= 2"); merging it into the `unless`
// rather than wrapping in an outer `if` keeps the output rubocop-clean.
void generateArgCheck(size_t i, const string &accessor, const string &guard = "") {
    fmt::print("      unless {2}arg{0}_type.valid?({1})\n"
               "        CallValidation.report_error(\n"
               "          method_sig,\n"
               "          method_sig.arg_types[{0}][1].error_message_for_obj({1}),\n"
               "          'Parameter',\n"
               "          method_sig.arg_types[{0}][0],\n"
               "          arg{0}_type,\n"
               "          {1},\n"
               "          caller_offset: -1\n"
               "        )\n"
               "      end\n",
               i, accessor, guard.empty() ? ""s : guard + " || ");
}

// Emits the shared return-value tail for the kwargs/block/optional-args wrappers.
// Unlike the fast/medium wrappers, these use one method for both `.void` and
// non-void sigs: a `nil` `return_type` means `.void` (return VOID, skip the check).
void generateReturnTail() {
    fmt::print("      if return_type.nil?\n"
               "        T::Private::Types::Void::VOID\n"
               "      else\n"
               "        unless return_type.valid?(return_value)\n"
               "          message = method_sig.effective_return_type.error_message_for_obj(return_value)\n"
               "          if message\n"
               "            CallValidation.report_error(\n"
               "              method_sig,\n"
               "              message,\n"
               "              'Return value',\n"
               "              nil,\n"
               "              method_sig.effective_return_type,\n"
               "              return_value,\n"
               "              caller_offset: -1\n"
               "            )\n"
               "          end\n"
               "        end\n"
               "        return_value\n"
               "      end\n");
}

// The kwargs path only covers the all-kwargs shape (`kwargs_path` requires
// `arg_types.empty?`), so only the zero-positional `kwargs0` wrapper exists.
void generateCreateValidatorKwargs(const Options &options) {
    fmt::print("  def self.create_validator_method_kwargs0(mod, original_method, method_sig, original_visibility, "
               "return_type, kwarg_types)\n");
    fmt::print("    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do "
               "|**kwargs, &blk|\n");
    fmt::print("      # This method is a manually sped-up version of more general code in `validate_call`\n");
    fmt::print("      # NOTE: like `validate_call`, we don't validate for missing or extra\n"
               "      # kwargs; the `bind_call` below takes care of that.\n");
    fmt::print("      kwargs.each do |name, val|\n"
               "        type = kwarg_types[name]\n"
               "        next unless type\n"
               "        next if type.valid?(val)\n"
               "        CallValidation.report_error(\n"
               "          method_sig,\n"
               "          type.error_message_for_obj(val),\n"
               "          'Parameter',\n"
               "          name,\n"
               "          type,\n"
               "          val,\n"
               "          caller_offset: 1\n"
               "        )\n"
               "      end\n");
    if (options.bindCall) {
        fmt::print("      return_value = original_method.bind_call(self, **kwargs, &blk)\n");
    } else {
        fmt::print("      return_value = original_method.bind(self).call(**kwargs, &blk)\n");
    }
    generateReturnTail();
    fmt::print("    end\n"
               "  end\n"
               "\n");
}

// Wrapper for a method with a required (non-nilable) block, unrolled per arity.
void generateCreateValidatorWithBlock(const Options &options, size_t arity) {
    fmt::print("  def self.create_validator_method_with_block{}(mod, original_method, method_sig, "
               "original_visibility, return_type, block_type",
               arity);
    for (size_t i = 0; i < arity; i++) {
        fmt::print(", arg{}_type", i);
    }
    fmt::print(")\n");

    fmt::print("    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do |");
    for (size_t i = 0; i < arity; i++) {
        fmt::print("arg{}, ", i);
    }
    fmt::print("&blk|\n");
    fmt::print("      # This method is a manually sped-up version of more general code in `validate_call`\n");

    for (size_t i = 0; i < arity; i++) {
        generateArgCheck(i, fmt::format("arg{}", i));
    }

    fmt::print("      if blk.nil?\n"
               "        CallValidation.report_error(\n"
               "          method_sig,\n"
               "          block_type.error_message_for_obj(blk),\n"
               "          'Block parameter',\n"
               "          method_sig.block_name,\n"
               "          block_type,\n"
               "          blk,\n"
               "          caller_offset: -1\n"
               "        )\n"
               "      end\n");

    fmt::print("      return_value = original_method");
    if (options.bindCall) {
        fmt::print(".bind_call(self, ");
    } else {
        fmt::print(".bind(self).call(");
    }
    for (size_t i = 0; i < arity; i++) {
        fmt::print("arg{}, ", i);
    }
    fmt::print("&blk)\n");

    generateReturnTail();
    fmt::print("    end\n"
               "  end\n"
               "\n");
}

// Wrapper for a method with optional positional args: `req` required positionals
// followed by `total - req` optional ones. Forwards through a ruby2_keywords-flagged
// `|*args, &blk|` splat so argument-collection semantics match the slow path.
void generateCreateValidatorOptionalArgs(const Options &options, size_t req, size_t total) {
    fmt::print("  def self.create_validator_method_optional_args{}_{}(mod, original_method, method_sig, "
               "original_visibility, return_type",
               req, total);
    for (size_t i = 0; i < total; i++) {
        fmt::print(", arg{}_type", i);
    }
    fmt::print(")\n");

    fmt::print("    T::Private::ClassUtils.def_with_visibility(mod, method_sig.method_name, original_visibility) do "
               "|*args, &blk|\n");
    fmt::print("      # This method is a manually sped-up version of more general code in `validate_call`\n");

    // Required args are always present; optional args are checked only when the
    // caller actually passed them. Defaults are never validated (matches the slow
    // path); `bind_call` enforces the arity bounds.
    for (size_t i = 0; i < req; i++) {
        generateArgCheck(i, fmt::format("args[{}]", i));
    }
    for (size_t i = req; i < total; i++) {
        // Skip the check when the optional arg was not passed. `i == 0` uses
        // `args.empty?` (rubocop prefers it over `args.length <= 0`).
        auto guard = i == 0 ? "args.empty?"s : fmt::format("args.length <= {}", i);
        generateArgCheck(i, fmt::format("args[{}]", i), guard);
    }

    fmt::print("      return_value = original_method");
    if (options.bindCall) {
        fmt::print(".bind_call(self, *args, &blk)\n");
    } else {
        fmt::print(".bind(self).call(*args, &blk)\n");
    }

    generateReturnTail();
    fmt::print("    end\n"
               "  end\n"
               "\n");
}

int generateCallValidation(const Options &options) {
    fmt::print("# frozen_string_literal: true\n"
               "# typed: false\n"
               "\n"
               "# DO NOT EDIT. This file is autogenerated. To regenerate, run:\n"
               "#\n"
               "#     bazel test //gems/sorbet-runtime:update_call_validation\n"
               "\n"
               "module T::Private::Methods::CallValidation\n");

    generateCreateValidatorFastDispatcher(ValidatorKind::Method, TypeKind::Simple);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Method, TypeKind::Simple, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::MethodSkipReturn, TypeKind::Simple);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::MethodSkipReturn, TypeKind::Simple, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Procedure, TypeKind::Simple);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Procedure, TypeKind::Simple, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Method, TypeKind::Complex);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Method, TypeKind::Complex, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::MethodSkipReturn, TypeKind::Complex);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::MethodSkipReturn, TypeKind::Complex, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Procedure, TypeKind::Complex);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Procedure, TypeKind::Complex, i);
    }

    // Specialized wrappers that cover three call shapes the fast/medium families
    // do not: keyword args, required (non-nilable) blocks, and optional positional
    // args. Dispatched from the matching trampolines in call_validation.rb.
    generateCreateValidatorKwargs(options);

    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorWithBlock(options, i);
    }

    for (size_t total = 1; total <= MAX_ARITY; total++) {
        for (size_t req = 0; req < total; req++) {
            generateCreateValidatorOptionalArgs(options, req, total);
        }
    }

    fmt::print("end\n");

    return 0;
}

int realmain(int argc, char **argv) {
    auto options = parseArgs(argc, argv);
    if (holds_alternative<int>(options)) {
        return get<int>(options);
    }

    return generateCallValidation(get<Options>(options));
}

} // namespace sorbet::sorbet_runtime::generate_call_validation

int main(int argc, char **argv) {
    return sorbet::sorbet_runtime::generate_call_validation::realmain(argc, argv);
}
