#include "common.h"
#include <cxxabi.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>

using namespace std;

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
        cerr << "Unhandled exception. Forcing a crash so ASAN prints a stack trace." << endl;
        *(volatile char *)nullptr = 0;
    }

    SetTerminateHandler() {
        set_terminate(&SetTerminateHandler::on_terminate);
    }
} SetTerminateHandler;

std::string demangle(const char *mangled) {
    int status;
    unique_ptr<char[], void (*)(void *)> result(abi::__cxa_demangle(mangled, 0, 0, &status), std::free);
    return result.get() ? std::string(result.get()) : "error occurred";
}