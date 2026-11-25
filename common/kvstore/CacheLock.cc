#include "common/kvstore/CacheLock.h"
#include "spdlog/fmt/fmt.h"

#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <utility>

using namespace std;

namespace sorbet {

namespace {

int acquire(const string &path, int operation) {
    auto lockPath = fmt::format("{}/{}", path, CacheLock::ACCESS_LOCK_FILE);
    int fd;
    do {
        fd = ::open(lockPath.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0664);
    } while (fd < 0 && errno == EINTR);
    if (fd < 0) {
        return -1;
    }

    while (flock(fd, operation) != 0) {
        if (errno == EINTR) {
            continue;
        }
        auto error = errno;
        ::close(fd);
        errno = error;
        return -1;
    }
    return fd;
}

} // namespace

CacheLock::CacheLock(int fd) : fd(fd) {}

CacheLock::CacheLock(CacheLock &&other) noexcept : fd(exchange(other.fd, -1)) {}

CacheLock &CacheLock::operator=(CacheLock &&other) noexcept {
    if (this != &other) {
        if (fd >= 0) {
            ::close(fd);
        }
        fd = exchange(other.fd, -1);
    }
    return *this;
}

CacheLock::~CacheLock() {
    if (fd >= 0) {
        ::close(fd);
    }
}

optional<CacheLock> CacheLock::acquireShared(const string &path, bool blocking) {
    auto fd = acquire(path, LOCK_SH | (blocking ? 0 : LOCK_NB));
    if (fd < 0) {
        return nullopt;
    }
    return CacheLock(fd);
}

optional<CacheLock> CacheLock::acquireExclusive(const string &path, bool blocking) {
    auto fd = acquire(path, LOCK_EX | (blocking ? 0 : LOCK_NB));
    if (fd < 0) {
        return nullopt;
    }
    return CacheLock(fd);
}

} // namespace sorbet
