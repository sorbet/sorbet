#ifdef __linux__
#include "common/common.h"
#include <cstdio>
#include <limits.h>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

string exec(string cmd);

string addr2line(const string program_name, void const *const *addr, int count) {
    auto addr2line_cmd = strprintf("atos -o %.256s", program_name.c_str());
    for (int i = 3; i < count; ++i) {
        addr2line_cmd = strprintf("%s %p", addr2line_cmd.c_str(), addr[i]);
    }

    return exec(addr2line_cmd);
}
string getProgramName() {
    char dest[512] = {}; // explicitly zero out

    if (readlink("/proc/self/exe", dest, PATH_MAX) < 0)
        return "<error>";

    string res(dest);
    return res;
}

#endif
