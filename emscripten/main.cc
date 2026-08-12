#include "absl/strings/match.h"
#include "common/EarlyReturnWithCode.h"
#include "main/lsp/wrapper.h"
#include "main/realmain.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
// so that we can compile this file with normal C++ compiler
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;

namespace {

unique_ptr<sorbet::realmain::lsp::SingleThreadedLSPWrapper> lspWrapper;

void initializeLSPWrapper(
    shared_ptr<sorbet::realmain::options::Options> options = make_shared<sorbet::realmain::options::Options>()) {
    if (lspWrapper) {
        return;
    }

    lspWrapper = sorbet::realmain::lsp::SingleThreadedLSPWrapper::create(string_view(), move(options));
    lspWrapper->enableAllExperimentalFeatures();
}

optional<string> applyParser(sorbet::realmain::options::Options &options, string_view parser) {
    if (parser == "prism") {
        options.cacheSensitiveOptions.usePrismParser = true;
        return nullopt;
    } else if (parser == "original") {
        options.cacheSensitiveOptions.usePrismParser = false;
        return nullopt;
    } else {
        return "Unknown `--parser` option: " + string(parser);
    }
}

optional<string> applyBooleanArgument(const string &argument, string_view name, bool &value) {
    if (argument == name) {
        value = true;
        return nullopt;
    } else {
        auto prefix = string(name) + "=";
        auto argumentValue = argument.substr(prefix.size());
        if (argumentValue == "true") {
            value = true;
            return nullopt;
        } else if (argumentValue == "false") {
            value = false;
            return nullopt;
        } else {
            return "Expected `" + prefix + "true` or `" + prefix + "false`";
        }
    }
}

optional<string> applyLSPOptions(sorbet::realmain::options::Options &options, const vector<string> &arguments) {
    constexpr string_view rbsComments = "--enable-experimental-rbs-comments";
    constexpr string_view methodModifiers = "--enable-experimental-method-modifiers";
    constexpr string_view parserPrefix = "--parser=";

    bool rbsEnabled = options.cacheSensitiveOptions.rbsEnabled;
    // No override means method modifiers follow the final RBS setting, matching the native CLI.
    optional<bool> methodModifiersOverride;

    for (size_t i = 0; i < arguments.size(); ++i) {
        const auto &argument = arguments[i];

        if (argument == rbsComments || absl::StartsWith(argument, string(rbsComments) + "=")) {
            if (auto error = applyBooleanArgument(argument, rbsComments, rbsEnabled)) {
                return error;
            }
            continue;
        }

        if (argument == methodModifiers || absl::StartsWith(argument, string(methodModifiers) + "=")) {
            bool enabled = false;
            if (auto error = applyBooleanArgument(argument, methodModifiers, enabled)) {
                return error;
            }
            methodModifiersOverride = enabled;
            continue;
        }

        if (argument == "--parser") {
            if (++i == arguments.size()) {
                return "Missing value for `--parser`";
            }
            if (auto error = applyParser(options, arguments[i])) {
                return error;
            }
            continue;
        }

        if (absl::StartsWith(argument, parserPrefix)) {
            if (auto error = applyParser(options, argument.substr(parserPrefix.size()))) {
                return error;
            }
        }
    }

    options.cacheSensitiveOptions.rbsEnabled = rbsEnabled;
    options.experimentalMethodModifiers = methodModifiersOverride.value_or(rbsEnabled);

    if (rbsEnabled && !options.cacheSensitiveOptions.usePrismParser) {
        return "RBS mode requires `--parser=prism`";
    }

    return nullopt;
}

void runSorbet(int argc, char *argv[]) {
    try {
        sorbet::realmain::realmain(argc, argv);
    } catch (sorbet::EarlyReturnWithCode &) {
        // Prevent handled Sorbet exits from aborting the wasm module and leaving the UI stuck.
    }
}

void typecheckString(const char *rubySrc) {
    const char *argv[] = {"sorbet", "--color=always", "--silence-dev-message", "-e", rubySrc};
    runSorbet(size(argv), const_cast<char **>(reinterpret_cast<const char **>(argv)));
}

} // namespace

extern "C" {
void EMSCRIPTEN_KEEPALIVE typecheck(const char *optionsJson) {
    rapidjson::Document options;
    options.Parse(optionsJson);
    if (options.HasParseError()) {
        fmt::print(stderr, "emscripten/main.cc: Failed to parse JSON from JavaScript: '{}'\n", optionsJson);
        fmt::print(stderr, "emscripten/main.cc: Falling back to assuming input was Ruby source.\n", optionsJson);
        return typecheckString(optionsJson);
    }

    if (!options.IsArray()) {
        fmt::print(stderr, "JSON from JavaScript was not an array: '{}'\n", optionsJson);
        fmt::print(stderr, "emscripten/main.cc: Falling back to assuming input was Ruby source.\n", optionsJson);
        return typecheckString(optionsJson);
    }

    vector<string> argStrings;
    for (rapidjson::SizeType i = 0; i < options.Size(); i++) {
        const auto &argI = options[i];
        if (!argI.IsString()) {
            fmt::print(stderr, "JSON from JavaScript was not a String at element {} of array: '{}'\n", i, optionsJson);
            fmt::print(stderr, "emscripten/main.cc: Falling back to assuming input was Ruby source.\n", optionsJson);
            return typecheckString(optionsJson);
        }

        argStrings.push_back(argI.GetString());
    }

    vector<char *> argCharStars;
    argCharStars.reserve(argStrings.size());
    for (size_t i = 0; i < argStrings.size(); ++i) {
        argCharStars.push_back(const_cast<char *>(argStrings[i].c_str()));
    }

    runSorbet(argCharStars.size(), argCharStars.data());
}

void EMSCRIPTEN_KEEPALIVE initializeLsp(const char *optionsJson) {
    rapidjson::Document options;
    options.Parse(optionsJson);
    if (options.HasParseError() || !options.IsArray()) {
        fmt::print(stderr, "emscripten/main.cc: LSP options were not a valid JSON array: '{}'. Using defaults.\n",
                   optionsJson);
        return;
    }

    vector<string> arguments;
    arguments.reserve(options.Size());
    for (rapidjson::SizeType i = 0; i < options.Size(); i++) {
        const auto &argument = options[i];
        if (!argument.IsString()) {
            fmt::print(stderr, "emscripten/main.cc: LSP option {} was not a string. Using defaults.\n", i);
            return;
        }
        arguments.emplace_back(argument.GetString());
    }

    auto lspOptions = make_shared<sorbet::realmain::options::Options>();
    if (auto error = applyLSPOptions(*lspOptions, arguments)) {
        fmt::print(stderr, "emscripten/main.cc: Invalid LSP options: {}. Using defaults.\n", *error);
        return;
    }

    initializeLSPWrapper(move(lspOptions));
}

void EMSCRIPTEN_KEEPALIVE lsp(void (*respond)(const char *), const char *message) {
    initializeLSPWrapper();

    auto responses = lspWrapper->getLSPResponsesFor(message);
    for (auto &response : responses) {
        respond(response->toJSON().c_str());
    }
}

int main(int argc, char **argv) {}
}
