#include "main/lsp/wrapper.h"
#include "main/realmain.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
// so that we can compile this file with normal C++ compiler
#define EMSCRIPTEN_KEEPALIVE
#endif

using namespace std;

typedef struct LSP {
    void (*respond)(const char *);
    sorbet::realmain::lsp::LSPWrapper *wrapper;
} LSP;

extern "C" {
void EMSCRIPTEN_KEEPALIVE typecheck(const char *rubySrc) {
    const char *argv[] = {"sorbet", "--color=always", "--silence-dev-message", "-e", rubySrc};
    sorbet::realmain::realmain(size(argv), const_cast<char **>(argv));
}

LSP *EMSCRIPTEN_KEEPALIVE lsp_initialize(void (*respond)(const char *)) {
    LSP *lsp = new LSP();
    lsp->respond = respond;
    lsp->wrapper = new sorbet::realmain::lsp::LSPWrapper();
    return lsp;
}

void EMSCRIPTEN_KEEPALIVE lsp_send(LSP *lsp, char *message) {
    auto responses = lsp->wrapper->getLSPResponsesFor(message);
    for (auto &response : responses) {
        lsp->respond(response->toJSON().c_str());
    }
}

int main(int argc, char **argv) {}
}
