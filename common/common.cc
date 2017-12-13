#include "common.h"
#include "os/os.h"
#include <array>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cxxabi.h>
#include <exception>
#include <execinfo.h>
#include <fstream>
#include <iostream>
#include <memory>
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

void ruby_typer::File::write(const char *filename, const vector<ruby_typer::u4> &data) {
    ofstream fout(filename, ios::out | ios::binary);
    if (!fout.good()) {
        throw ruby_typer::FileNotFoundException();
    }
    fout.write((const char *)data.data(), data.size() * sizeof(ruby_typer::u4));

    return;
}

string ruby_typer::Strings::escapeCString(absl::string_view what) {
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
        if (j == sizeof(escaped)) {
            buf << c;
        }
    }

    return buf.str();
}

absl::string_view ruby_typer::File::getFileName(const absl::string_view path) {
    std::size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

absl::string_view ruby_typer::File::getExtension(const absl::string_view path) {
    std::size_t found = path.find_last_of(".");
    if (found == absl::string_view::npos) {
        return absl::string_view();
    }
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
        ruby_typer::Error::print_backtrace();
    }

    SetTerminateHandler() {
        set_terminate(&SetTerminateHandler::on_terminate);
    }
} SetTerminateHandler;

string exec(string cmd) {
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (feof(pipe.get()) == 0) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            result += buffer.data();
        }
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
    char **messages = (char **)nullptr;
    string program_name = getProgramName();

    trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
    messages = backtrace_symbols(stack_traces, trace_size);

    string res = addr2line(program_name, stack_traces, trace_size);
    filter_unnecessary(res);
    fprintf(stderr, "Backtrace:\n%s", res.c_str());

    if (messages != nullptr) {
        free(messages);
    }
}

string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, nullptr, nullptr, &status), free);
    return result.get() != nullptr ? string(result.get()) : "error occurred";
}
