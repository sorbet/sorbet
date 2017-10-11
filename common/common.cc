#include "common.h"
#include <fstream>

std::string ruby_typer::File::read(const char *filename) {
    std::ifstream fin(filename);
    // Determine the file length
    ruby_typer::Error::check(fin.good());
    std::string src;
    fin.seekg(0, std::ios::end);
    src.reserve(fin.tellg());
    fin.seekg(0, std::ios::beg);

    src.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    return src;
}
