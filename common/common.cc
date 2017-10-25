#include "common.h"
#include <exception>
#include <fstream>
#include <iostream>

using namespace std;

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
