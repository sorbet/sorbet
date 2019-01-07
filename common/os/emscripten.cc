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

#endif
