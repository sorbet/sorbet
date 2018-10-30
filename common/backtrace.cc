#include "common.h"

#ifndef EMSCRIPTEN
#include <execinfo.h>
#include <string>

using namespace std;

#define MAX_STACK_FRAMES 128
void *stack_traces[MAX_STACK_FRAMES];

void filter_unnecessary(string &out) {
    string::size_type i = 0;
    string::size_type j = 0;
    vector<string> patterns{"typecase.h:", "__functional_base:", "functional:"};

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

void sorbet::Exception::print_backtrace() noexcept {
    int trace_size = 0;
    auto **messages = (char **)nullptr;
    string program_name = getProgramName();

    trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stack_traces, trace_size);

    string res = addr2line(program_name, stack_traces, trace_size);
    filter_unnecessary(res);
    fatalLogger->error("Backtrace:\n{}", res.c_str());

    if (messages != nullptr) {
        free(messages);
    }
}

#else

void sorbet::Exception::print_backtrace() noexcept {}
#endif
