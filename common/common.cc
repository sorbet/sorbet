#include "common.h"
#include <exception>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>
#ifdef __APPLE__
#include <array>
#include <cstdio>
#include <err.h>
#include <execinfo.h>
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#include <memory>
#include <stdint.h>
#include <unistd.h>
#endif

using namespace std;

#define MAX_STACK_FRAMES 128
static void *stack_traces[MAX_STACK_FRAMES];
char addr2line_cmd[1024 * MAX_STACK_FRAMES] = {0};
static char program_name[256];

string ruby_typer::File::read(const char *filename) {
    ifstream fin(filename);
    // Determine the file length
    ruby_typer::Error::check(fin.good());
    string src;
    fin.seekg(0, ios::end);
    src.reserve(fin.tellg());
    fin.seekg(0, ios::beg);

    src.assign((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    return src;
}

class SetTerminateHandler {
public:
    static void on_terminate() {
        ruby_typer::Error::print_backtrace();
    }

    SetTerminateHandler() {
        set_terminate(&SetTerminateHandler::on_terminate);
    }
} SetTerminateHandler;

void ruby_typer::Error::print_backtrace() {

#ifdef __APPLE__

string exec(const char *cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

string addr2line(char const *const program_name, void const *const *addr, int count) {
    char addr2line_cmd[2048] = {0};

    int s = sprintf(addr2line_cmd, "atos -o %.256s", program_name);
    for (int i = 3; i < count; ++i) {
        s += sprintf(addr2line_cmd + s, " %p", addr[i]);
    }
    addr2line_cmd[s] = 0;
    //    printf(addr2line_cmd);

    return exec(addr2line_cmd);
}
#endif

void filter_unececary(string &out) {
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

void ruby_typer::Error::print_backtrace() {
#ifdef __APPLE__
    int trace_size = 0;
    char **messages = (char **)NULL;
    uint32_t sz = 256;
    _NSGetExecutablePath(program_name, &sz);

    trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stack_traces, trace_size);

    string res = addr2line(program_name, stack_traces, trace_size);
    filter_unececary(res);
    fprintf(stderr, "%s", res.c_str());

    if (messages) {
        free(messages);
    }
#endif
}
