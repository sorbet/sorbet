#ifdef __linux__
#include "common/common.h"
#include <climits>
#include <csignal>
#include <cstdio>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

extern "C" {
// If we are linked against the LLVM sanitizers, this symbol will be
// replaced with the definition from the sanitizer runtime
void __attribute__((weak)) __sanitizer_symbolize_pc(void *pc, const char *fmt, char *out_buf, size_t out_buf_size) {
    snprintf(out_buf, out_buf_size, "%p", pc);
}
}

string exec(string cmd);

string addr2line(const string program_name, void const *const *addr, int count) {
    stringstream os;
    for (int i = 3; i < count; ++i) {
        char buf[4096];
        __sanitizer_symbolize_pc(const_cast<void *>(addr[i]), "%p in %f %s:%l:%c", buf, sizeof(buf));
        os << "  #" << i << " " << buf << "\n";
    }
    return os.str();
}
string getProgramName() {
    char dest[512] = {}; // explicitly zero out

    if (readlink("/proc/self/exe", dest, PATH_MAX) < 0) {
        return "<error>";
    }

    string res(dest);
    return res;
}

#endif
