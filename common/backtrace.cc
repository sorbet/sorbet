#include "common.h"

#ifndef EMSCRIPTEN
#include <execinfo.h>
#include <string>

using namespace std;

#define MAX_STACK_FRAMES 128
void *stackTraces[MAX_STACK_FRAMES];

void filter_unnecessary(string &out) {
    string::size_type i = 0;
    string::size_type j = 0;
    string_view patterns[] = {"typecase.h:"sv, "__functional_base:"sv, "functional:"sv};

    while (i < out.length()) {
        i = out.find('\n', i);
        if (i == string::npos) {
            break;
        }
        j = out.find('\n', i + 1);
        if (j == string::npos) {
            break;
        }
        bool found = false;
        string substr(out, i, j - i);
        for (auto &subp : patterns) {
            found = found || (substr.find(subp) != string::npos);
        }
        if (found) {
            out.erase(i, j - i);
        } else {
            i = i + 1;
        }
    }
}

void sorbet::Exception::printBacktrace() noexcept {
    int traceSize = 0;
    auto **messages = (char **)nullptr;
    string programName = getProgramName();

    traceSize = backtrace(stackTraces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stackTraces, traceSize);

    string res = addr2line(programName, stackTraces, traceSize);
    filter_unnecessary(res);
    fatalLogger->error("Backtrace:\n{}", res.c_str());

    if (messages != nullptr) {
        free(messages);
    }
}

#else

void sorbet::Exception::printBacktrace() noexcept {}
#endif // ifndef EMSCRIPTEN
void sorbet::Exception::failInFuzzer() noexcept {
    if (fuzz_mode) {
        __builtin_trap();
    }
}
