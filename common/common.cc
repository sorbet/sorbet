#include "common.h"
#include "os/os.h"
#include <array>
#include <cstdarg>
#include <cxxabi.h>
#include <exception>
#include <execinfo.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <vector>

using namespace std;

#define MAX_STACK_FRAMES 128
static void *stack_traces[MAX_STACK_FRAMES];

string ruby_typer::File::read(const char *filename) {
    ifstream fin(filename);
    if (!fin.good()) {
        throw ruby_typer::FileNotFoundException();
    }
    // Determine the file length
    string src;
    fin.seekg(0, ios::end);
    src.reserve(fin.tellg());
    fin.seekg(0, ios::beg);

    src.assign((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    return src;
}

string ruby_typer::Strings::escape(string what) {
    char escaped[] = {'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\"'};
    char non_escaped[] = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\"'};
    static_assert(sizeof(escaped) == sizeof(non_escaped), "???");
    stringstream buf;
    for (char c : what) {
        int j;
        for (j = 0; j < sizeof(escaped); j++) {
            if (c == escaped[j]) {
                buf << '\\';
                buf << non_escaped[j];
                break;
            }
        }
        if (j == sizeof(escaped))
            buf << c;
    }

    return buf.str();
}

std::string ruby_typer::File::getFileName(const std::string path) {
    std::size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

string strprintf(const char *format, va_list vlist) {
    char *buf = nullptr;
    int ret = vasprintf(&buf, format, vlist);
    ruby_typer::Error::check(ret >= 0);
    string str = buf;
    free(buf);
    return str;
}

string strprintf(const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    auto str = strprintf(format, vlist);
    va_end(vlist);
    return str;
}

class SetTerminateHandler {
public:
    static void on_terminate() {
#ifdef __APPLE__
        // does not provide nice stack traces on linux.
        ruby_typer::Error::print_backtrace();
#else
        cerr << "Unhandled exception. Forcing a crash so ASAN prints a stack trace." << endl;
        raise(SIGSEGV);
#endif
    }

    SetTerminateHandler() {
        set_terminate(&SetTerminateHandler::on_terminate);
    }
} SetTerminateHandler;

string exec(string cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
        throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

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

void ruby_typer::Error::print_backtrace() {
    int trace_size = 0;
    char **messages = (char **)NULL;
    string program_name = getProgramName();

    trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stack_traces, trace_size);

    string res = addr2line(program_name, stack_traces, trace_size);
    filter_unnecessary(res);
    fprintf(stderr, "%s", res.c_str());

    if (messages) {
        free(messages);
    }
}

string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, 0, 0, &status), free);
    return result.get() ? string(result.get()) : "error occurred";
}
