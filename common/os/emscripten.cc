#ifdef EMSCRIPTEN
#include <string>

using namespace std;

bool stopInDebugger() {
    return false;
}

string getProgramName() {
    return "sorbet";
}

bool setCurrentThreadName(const std::string &name) {
    return false;
}

#endif
