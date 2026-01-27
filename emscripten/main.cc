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

unique_ptr<sorbet::realmain::lsp::SingleThreadedLSPWrapper> wrapper;

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
    if (wrapper) {
        return;
    }

    rapidjson::Document options;
    options.Parse(optionsJson);
    if (options.HasParseError() || !options.IsArray()) {
        fmt::print(stderr, "emscripten/main.cc: LSP options were not a valid JSON array: '{}'. Using defaults.\n",
                   optionsJson);
        return;
    }

    auto opts = make_shared<sorbet::realmain::options::Options>();
    optional<bool> methodModifiersEnabled;

    for (rapidjson::SizeType i = 0; i < options.Size(); i++) {
        const auto &value = options[i];
        if (!value.IsString()) {
            fmt::print(stderr, "emscripten/main.cc: LSP option {} was not a string. Using defaults.\n", i);
            return;
        }

        string_view argument = value.GetString();
        if (argument == "--parser=prism") {
            opts->cacheSensitiveOptions.usePrismParser = true;
        } else if (argument == "--parser=original") {
            opts->cacheSensitiveOptions.usePrismParser = false;
        } else if (argument == "--parser") {
            if (++i == options.Size() || !options[i].IsString()) {
                fmt::print(stderr, "emscripten/main.cc: Missing value for `--parser`. Using defaults.\n");
                return;
            }

            string_view parser = options[i].GetString();
            if (parser == "prism") {
                opts->cacheSensitiveOptions.usePrismParser = true;
            } else if (parser == "original") {
                opts->cacheSensitiveOptions.usePrismParser = false;
            }
        } else if (argument == "--enable-experimental-rbs-comments" ||
                   argument == "--enable-experimental-rbs-comments=true") {
            opts->cacheSensitiveOptions.rbsEnabled = true;
        } else if (argument == "--enable-experimental-rbs-comments=false") {
            opts->cacheSensitiveOptions.rbsEnabled = false;
        } else if (argument == "--enable-experimental-method-modifiers" ||
                   argument == "--enable-experimental-method-modifiers=true") {
            methodModifiersEnabled = true;
        } else if (argument == "--enable-experimental-method-modifiers=false") {
            methodModifiersEnabled = false;
        }
    }

    opts->experimentalMethodModifiers =
        methodModifiersEnabled.value_or(static_cast<bool>(opts->cacheSensitiveOptions.rbsEnabled));
    if (opts->cacheSensitiveOptions.rbsEnabled && !opts->cacheSensitiveOptions.usePrismParser) {
        fmt::print(stderr, "emscripten/main.cc: RBS mode requires `--parser=prism`. Using defaults.\n");
        return;
    }

    wrapper = sorbet::realmain::lsp::SingleThreadedLSPWrapper::create(string_view(), move(opts));
    wrapper->enableAllExperimentalFeatures();
}

void EMSCRIPTEN_KEEPALIVE lsp(void (*respond)(const char *), const char *message) {
    if (!wrapper) {
        wrapper = sorbet::realmain::lsp::SingleThreadedLSPWrapper::create();
        wrapper->enableAllExperimentalFeatures();
    }

    auto responses = wrapper->getLSPResponsesFor(message);
    for (auto &response : responses) {
        respond(response->toJSON().c_str());
    }
}

int main(int argc, char **argv) {}
}
