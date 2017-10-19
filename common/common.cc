#include "common.h"
#include <fstream>

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
