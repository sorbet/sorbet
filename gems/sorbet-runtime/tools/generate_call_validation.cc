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

string_view validatorKindToString(ValidatorKind kind) {
    switch (kind) {
        case ValidatorKind::Method:
            return "method"sv;
        case ValidatorKind::Procedure:
            return "procedure"sv;
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

void generateCreateValidatorFastDispatcher(ValidatorKind kind) {
    auto kindString = validatorKindToString(kind);
    fmt::print("  def self.create_validator_{}_fast(mod, original_method, method_sig)\n", kindString);
    switch (kind) {
        case ValidatorKind::Method:
            fmt::print("    if method_sig.return_type.is_a?(T::Private::Types::Void)\n");
            fmt::print("      raise 'Should have used create_validator_procedure_fast'\n");
            fmt::print("    end\n");
            break;
        case ValidatorKind::Procedure:
            break;
    }

    const char *returnTypeArg;
    switch (kind) {
        case ValidatorKind::Method:
            returnTypeArg = ", method_sig.return_type.raw_type";
            break;
        case ValidatorKind::Procedure:
            returnTypeArg = "";
            break;
    }
    fmt::print("    # trampoline to reduce stack frame size\n");
    for (size_t arity = 0; arity <= MAX_ARITY; arity++) {
        if (arity == 0) {
            fmt::print("    if method_sig.arg_types.empty?\n");
        } else {
            fmt::print("    elsif method_sig.arg_types.length == {}\n", arity);
        }

        fmt::print("      create_validator_{}_fast{}(mod, original_method, method_sig{}", kindString, arity,
                   returnTypeArg);
        for (size_t i = 0; i < arity; i++) {
            fmt::print(",\n");
            fmt::print("                                    method_sig.arg_types[{}][1].raw_type", i);
        }
        fmt::print(")\n");
    }
    fmt::print("    else\n");
    fmt::print("      raise 'should not happen'\n");
    fmt::print("    end\n");
    fmt::print("  end\n");
    fmt::print("\n");
}

void generateCreateValidatorFast(const Options &options, ValidatorKind kind, size_t arity) {
    const char *returnTypeArg;
    switch (kind) {
        case ValidatorKind::Method:
            returnTypeArg = ", return_type";
            break;
        case ValidatorKind::Procedure:
            returnTypeArg = "";
            break;
    }
    fmt::print("  def self.create_validator_{}_fast{}(mod, original_method, method_sig{}", validatorKindToString(kind),
               arity, returnTypeArg);
    for (size_t i = 0; i < arity; i++) {
        fmt::print(", arg{}_type", i);
    }
    fmt::print(")\n");

    fmt::print("    mod.send(:define_method, method_sig.method_name) do |");
    for (size_t i = 0; i < arity; i++) {
        fmt::print("arg{}, ", i);
    }
    fmt::print("&blk|\n");

    fmt::print("      # This block is called for every `sig`. It's critical to keep it fast and\n");
    fmt::print("      # reduce number of allocations that happen here.\n");
    fmt::print("      # This method is a manually sped-up version of more general code in `validate_call`\n");
    fmt::print("      T::Profile.typecheck_sample_attempts -= 1\n");
    fmt::print("      should_sample = T::Profile.typecheck_sample_attempts == 0\n");
    fmt::print("      if should_sample\n");
    fmt::print("        T::Profile.typecheck_sample_attempts = T::Profile::SAMPLE_RATE\n");
    fmt::print("        T::Profile.typecheck_samples += 1\n");
    fmt::print("        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)\n");
    fmt::print("      end\n");
    fmt::print("\n");

    for (size_t i = 0; i < arity; i++) {
        fmt::print("      unless arg{0}.is_a?(arg{0}_type)\n", i);
        fmt::print("        CallValidation.report_error(\n");
        fmt::print("          method_sig,\n");
        fmt::print("          method_sig.arg_types[{0}][1].error_message_for_obj(arg{0}),\n", i);
        fmt::print("          'Parameter',\n");
        fmt::print("          method_sig.arg_types[{0}][0],\n", i);
        fmt::print("          arg{0}_type,\n", i);
        fmt::print("          arg{0},\n", i);
        fmt::print("          caller_offset: -1\n");
        fmt::print("        )\n");
        fmt::print("      end\n");
        fmt::print("\n");
    }

    fmt::print("      if should_sample\n");
    fmt::print("        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)\n");
    fmt::print("      end\n");
    fmt::print("\n");

    fmt::print("      # The following line breaks are intentional to show nice pry message\n");
    for (size_t i = 0; i < 10; i++) {
        fmt::print("\n");
    }
    fmt::print("      # PRY note:\n");
    fmt::print("      # this code is sig validation code.\n");
    fmt::print("      # Please issue `finish` to step out of it\n");
    fmt::print("\n");

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

    switch (kind) {
        case ValidatorKind::Procedure:
            fmt::print("      T::Private::Types::Void::VOID\n");
            break;

        case ValidatorKind::Method:
            fmt::print("      if should_sample\n");
            fmt::print("        t1 = Process.clock_gettime(Process::CLOCK_MONOTONIC)\n");
            fmt::print("      end\n");
            fmt::print("\n");

            fmt::print("      unless return_value.is_a?(return_type)\n");
            fmt::print("        message = method_sig.return_type.error_message_for_obj(return_value)\n");
            fmt::print("        if message\n");
            fmt::print("          CallValidation.report_error(\n");
            fmt::print("            method_sig,\n");
            fmt::print("            message,\n");
            fmt::print("            'Return value',\n");
            fmt::print("            nil,\n");
            fmt::print("            method_sig.return_type,\n");
            fmt::print("            return_value,\n");
            fmt::print("            caller_offset: -1\n");
            fmt::print("          )\n");
            fmt::print("        end\n");
            fmt::print("      end\n");
            fmt::print("      if should_sample\n");
            fmt::print(
                "        T::Profile.typecheck_duration += (Process.clock_gettime(Process::CLOCK_MONOTONIC) - t1)\n");
            fmt::print("      end\n");
            fmt::print("      return_value\n");
            break;
    }

    fmt::print("    end\n");
    fmt::print("  end\n");
    fmt::print("\n");
}

int generateCallValidation(const Options &options) {
    fmt::print("# frozen_string_literal: true\n");
    fmt::print("# typed: false\n");
    fmt::print("\n");
    fmt::print("# DO NOT EDIT. This file is autogenerated. To regenerate, run:\n");
    fmt::print("#\n");
    fmt::print("#     bazel test //gems/sorbet-runtime:update_call_validation\n");
    fmt::print("\n");
    fmt::print("module T::Private::Methods::CallValidation\n");

    generateCreateValidatorFastDispatcher(ValidatorKind::Method);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Method, i);
    }

    generateCreateValidatorFastDispatcher(ValidatorKind::Procedure);
    for (size_t i = 0; i <= MAX_ARITY; i++) {
        generateCreateValidatorFast(options, ValidatorKind::Procedure, i);
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
