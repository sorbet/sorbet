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

void initializeLSPWrapper() {
    if (lspWrapper) {
        return;
    }

    lspWrapper = sorbet::realmain::lsp::SingleThreadedLSPWrapper::create();
    lspWrapper->enableAllExperimentalFeatures();
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

    for (rapidjson::SizeType i = 0; i < options.Size(); i++) {
        if (!options[i].IsString()) {
            fmt::print(stderr, "emscripten/main.cc: LSP option {} was not a string. Using defaults.\n", i);
            return;
        }
    }

    initializeLSPWrapper();
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
