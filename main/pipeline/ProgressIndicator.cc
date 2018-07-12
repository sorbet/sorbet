#include "ProgressIndicator.h"

#include <utility>

using namespace std;

void ProgressIndicator::reportProgress(int current) {
    ENFORCE(this_thread::get_id() == outputThreadId);
    auto currentTime = getCurrentTimeMillis();
    if (enabled && (lastReported - currentTime + REPORTING_INTERVAL() <= chrono::seconds{0})) {
        lastReported = currentTime;
        progressbar_update(progress.get(), current);
    }
}

ProgressIndicator::~ProgressIndicator() {
    if (enabled) {
        progressbar_finish(progress.release());
    }
}

ProgressIndicator::ProgressIndicator(bool enabled, string name, int progressExpected)
    : progressExpected(progressExpected), name(move(name)), enabled(enabled) {
    outputThreadId = this_thread::get_id();
    if (enabled) {
        progress.reset(progressbar_new(this->name.c_str(), this->progressExpected));
        lastReported = getCurrentTimeMillis();
    }
}
