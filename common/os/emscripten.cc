#ifdef EMSCRIPTEN

#include <string>

using namespace std;

bool stopInDebugger() {
    return false;
}

string getProgramName() {
    return "sorbet";
}

bool setCurrentThreadName(string_view name) {
    return false;
}

void initializeSymbolizer(char *argv0) {
    // Our version of emscripten doesn't provide the offset converter that the debugging library
    // from absl is looking for, so we can't call absl::InitializeSymbolizer here, otherwise it will
    // log an INFO message to stdout when it starts up suggesting how to work around this.
    // absl::InitializeSymbolizer(argv0);
}

#endif
