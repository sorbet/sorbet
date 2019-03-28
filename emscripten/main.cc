#include "main/lsp/wrapper.h"
#include "main/realmain.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
// so that we can compile this file with normal C++ compiler
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;

extern "C" {
void EMSCRIPTEN_KEEPALIVE typecheck(const char *rubySrc) {
    const char *argv[] = {"sorbet", "--color=always", "--silence-dev-message", "-e", rubySrc};
    sorbet::realmain::realmain(size(argv), const_cast<char **>(argv));
}

void EMSCRIPTEN_KEEPALIVE lsp(void (*respond)(const char *), const char *message) {
    static sorbet::realmain::lsp::LSPWrapper *wrapper;
    if (!wrapper) {
        wrapper = new sorbet::realmain::lsp::LSPWrapper();
        wrapper->enableAllExperimentalFeatures();
    }

    auto responses = wrapper->getLSPResponsesFor(message);
    for (auto &response : responses) {
        respond(response->toJSON().c_str());
    }
    wrapper->freeJSONObjects();
}

int main(int argc, char **argv) {}
}
