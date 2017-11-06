#ifdef __APPLE__
#include "common/common.h"
#include <cstdio>
#include <mach-o/dyld.h> /* _NSGetExecutablePath */
#include <string>

using namespace std;

string exec(string cmd);

string addr2line(const string program_name, void const *const *addr, int count) {
    auto addr2line_cmd = strprintf("atos -o %.256s", program_name.c_str());
    for (int i = 3; i < count; ++i) {
        addr2line_cmd = strprintf("%s %p", addr2line_cmd.c_str(), addr[i]);
    }

    //    printf("%s\n", addr2line_cmd.c_str());

    return exec(addr2line_cmd);
}
string getProgramName() {
    char buf[512];
    uint32_t sz = 512;
    _NSGetExecutablePath(buf, &sz);
    string res(buf);
    return res;
}

#endif
