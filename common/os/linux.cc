#ifdef __linux__
#include <cstdio>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

std::string exec(const char *cmd);

std::string addr2line(const std::string program_name, void const *const *addr, int count) {
    /*
        char addr2line_cmd[2048] = {0};

        int s = sprintf(addr2line_cmd, "addr2line -f -p -e %.256s", program_name.c_str());
        for (int i = 3; i < count; ++i) {
            s += sprintf(addr2line_cmd + s, " %p", addr[i]);
        }
        addr2line_cmd[s] = 0;
        //    printf(addr2line_cmd);

        return exec(addr2line_cmd);
    */
}
std::string getProgramName() {
    /*
        char path[512];
        char dest[512];
        memset(dest,0,sizeof(dest));// readlink does not terminate the string...
        struct stat info;
        pid_t pid = getpid();
        sprintf(path, "/proc/%d/exe", pid);
        readlink(path, dest, PATH_MAX)
        std::string res(dest);
        return res;
    */
}
#endif