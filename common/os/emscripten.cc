#ifdef EMSCRIPTEN
#include <string>

using namespace std;

bool stopInDebugger() {
    return false;
}

string getProgramName() {
    return "sorbet";
}
#endif
