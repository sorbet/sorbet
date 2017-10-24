#ifdef __APPLE__
#include <cstdio>
#include <string>
#include <mach-o/dyld.h> /* _NSGetExecutablePath */

std::string exec(const char *cmd);

std::string addr2line(const std::string program_name, void const *const *addr, int count) {
    char addr2line_cmd[2048] = {0};

    int s = sprintf(addr2line_cmd, "atos -o %.256s", program_name.c_str());
    for (int i = 3; i < count; ++i) {
        s += sprintf(addr2line_cmd + s, " %p", addr[i]);
    }
    addr2line_cmd[s] = 0;
    //    printf(addr2line_cmd);

    return exec(addr2line_cmd);
}
std::string getProgramName() {
    char buf[512];
    uint32_t sz;
    _NSGetExecutablePath(buf, &sz);
    std::string res(buf);
    return res;
}

#endif