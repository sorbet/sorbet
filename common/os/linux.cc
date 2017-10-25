#ifdef __linux__
#include "common/common.h"
#include <cstdio>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

std::string exec(std::string cmd);

std::string addr2line(const std::string program_name, void const *const *addr, int count) {

    auto addr2line_cmd = strprintf("atos -o %.256s", program_name.c_str());
    for (int i = 3; i < count; ++i) {
        addr2line_cmd = strprintf("%s %p", addr2line_cmd.c_str(), addr[i]);
    }

    return exec(addr2line_cmd);
}
std::string getProgramName() {
    char dest[512] = {}; // explicitly zero out

    readlink("/proc/self/exe", dest, PATH_MAX);
    std::string res(dest);
    return res;
}

#endif
