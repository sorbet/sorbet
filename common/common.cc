#include "common.h"
#include "Error.h"
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

namespace {
shared_ptr<spdlog::logger> makeFatalLogger() {
    auto alreadyExists = spdlog::get("fatalFallback");
    if (!alreadyExists) {
        return spdlog::stdout_color_mt("fatalFallback");
    }
    return alreadyExists;
}
} // namespace
shared_ptr<spdlog::logger> sorbet::fatalLogger = makeFatalLogger();

string sorbet::FileOps::read(const absl::string_view filename) {
    string fileNameStr(filename.data(), filename.size());
    ifstream fin(fileNameStr);
    if (!fin.good()) {
        throw sorbet::FileNotFoundException();
    }
    // Determine the file length
    string src;
    fin.seekg(0, ios::end);
    src.reserve(fin.tellg());
    fin.seekg(0, ios::beg);

    src.assign((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    return src;
}

void sorbet::FileOps::write(const absl::string_view filename, const vector<sorbet::u1> &data) {
    string fileNameStr(filename.data(), filename.size());
    ofstream fout(fileNameStr, ios::out | ios::binary);
    if (!fout.good()) {
        throw sorbet::FileNotFoundException();
    }
    fout.write((const char *)data.data(), data.size());
}

void sorbet::FileOps::write(const absl::string_view filename, const absl::string_view text) {
    string fileNameStr(filename.data(), filename.size());
    ofstream fout(fileNameStr);
    if (!fout.good()) {
        throw sorbet::FileNotFoundException();
    }
    fout << text;
}

absl::string_view sorbet::FileOps::getFileName(const absl::string_view path) {
    std::size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

absl::string_view sorbet::FileOps::getExtension(const absl::string_view path) {
    std::size_t found = path.find_last_of(".");
    if (found == absl::string_view::npos) {
        return absl::string_view();
    }
    return path.substr(found + 1);
}

string strprintf(const char *format, va_list vlist) {
    char *buf = nullptr;
    int ret = vasprintf(&buf, format, vlist);
    ENFORCE(ret >= 0, "vasprintf failed");
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
        sorbet::Error::print_backtrace();
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

namespace {
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
} // namespace

void sorbet::Error::print_backtrace() noexcept {
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

string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, nullptr, nullptr, &status), free);
    return result.get() != nullptr ? string(result.get()) : "error occurred";
}
