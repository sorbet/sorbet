#include "common/common.h"
#include "common/Exception.h"
#include "os/os.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <array>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cxxabi.h>
#include <exception>
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

string sorbet::FileOps::read(string_view filename) {
    std::FILE *fp = std::fopen((string(filename)).c_str(), "rb");
    if (fp) {
        std::string contents;
        std::fseek(fp, 0, SEEK_END);
        contents.resize(std::ftell(fp));
        std::rewind(fp);
        auto readBytes = std::fread(&contents[0], 1, contents.size(), fp);
        std::fclose(fp);
        if (readBytes != contents.size()) {
            // Error reading file?
            throw sorbet::FileNotFoundException();
        }
        return contents;
    }
    throw sorbet::FileNotFoundException();
}

void sorbet::FileOps::write(string_view filename, const vector<sorbet::u1> &data) {
    std::FILE *fp = std::fopen(string(filename).c_str(), "wb");
    if (fp) {
        std::fwrite(data.data(), sizeof(sorbet::u1), data.size(), fp);
        std::fclose(fp);
        return;
    }
    throw sorbet::FileNotFoundException();
}

void sorbet::FileOps::write(string_view filename, string_view text) {
    std::FILE *fp = std::fopen(string(filename).c_str(), "w");
    if (fp) {
        std::fwrite(text.data(), sizeof(char), text.size(), fp);
        std::fclose(fp);
        return;
    }
    throw sorbet::FileNotFoundException();
}

string_view sorbet::FileOps::getFileName(string_view path) {
    size_t found = path.find_last_of("/\\");
    return path.substr(found + 1);
}

string_view sorbet::FileOps::getExtension(string_view path) {
    size_t found = path.find_last_of(".");
    if (found == string_view::npos) {
        return string_view();
    }
    return path.substr(found + 1);
}

class SetTerminateHandler {
public:
    static void on_terminate() {
        sorbet::Exception::print_backtrace();
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

string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, nullptr, nullptr, &status), free);
    return result.get() != nullptr ? string(result.get()) : "error occurred";
}
