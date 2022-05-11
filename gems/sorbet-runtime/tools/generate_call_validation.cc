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
            fmt::print("    if method_sig.return_type.is_a?(T::Private::Types::Void)\n"
                       "      raise 'Should have used create_validator_procedure_{}'\n"
                       "    end\n",
                       typeString);
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
    for (size_t arity = 0; arity <= MAX_ARITY; arity++) {
        if (arity == 0) {
            fmt::print("    if method_sig.arg_types.empty?\n");
        } else {
            fmt::print("    elsif method_sig.arg_types.length == {}\n", arity);
        }

        fmt::print("      create_validator_{}_{}{}(mod, original_method, method_sig, original_visibility", kindString,
                   typeString, arity);

        if (kind == ValidatorKind::Method) {
            fmt::print(", method_sig.return_type{}", rawTypeMethodCall);
        }

        for (size_t i = 0; i < arity; i++) {
            fmt::print(",\n");
            fmt::print("                                    method_sig.arg_types[{}][1]{}", i, rawTypeMethodCall);
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

    const char *returnValueTypecheck;
    switch (type) {
        case TypeKind::Simple:
            returnValueTypecheck = "return_value.is_a?(return_type)";
            break;
        case TypeKind::Complex:
            returnValueTypecheck = "return_type.valid?(return_value)";
            break;
    }

    switch (kind) {
        case ValidatorKind::Procedure:
            fmt::print("      T::Private::Types::Void::VOID\n");
            break;

        case ValidatorKind::Method:
            fmt::print("      unless {}\n"
                       "        message = method_sig.return_type.error_message_for_obj(return_value)\n"
                       "        if message\n"
                       "          CallValidation.report_error(\n"
                       "            method_sig,\n"
                       "            message,\n"
                       "            'Return value',\n"
                       "            nil,\n"
                       "            method_sig.return_type,\n"
                       "            return_value,\n"
                       "            caller_offset: -1\n"
                       "          )\n"
                       "        end\n"
                       "      end\n"
                       "      return_value\n",
                       returnValueTypecheck);
            break;
    }

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

    generateCreateValidatorFastDispatcher(ValidatorKind::Procedure, TypeKind::Simple);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Procedure, TypeKind::Simple, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Method, TypeKind::Complex);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Method, TypeKind::Complex, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Procedure, TypeKind::Complex);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Procedure, TypeKind::Complex, i);
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
