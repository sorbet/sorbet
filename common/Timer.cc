#include "Timer.h"
using namespace std;
long timespec_delta(struct timespec *start, struct timespec *stop) {
    return (stop->tv_sec - start->tv_sec) * 1000000000 + stop->tv_nsec - start->tv_nsec;
}

Timer::Timer(shared_ptr<spdlog::logger> log, const string &msg) : log(log), msg(msg) {
    clock_gettime(CLOCK_REALTIME, &begin);
}

Timer::~Timer() {
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    log->debug("{}: {}ms", this->msg, timespec_delta(&begin, &end) / 1000000);
}
