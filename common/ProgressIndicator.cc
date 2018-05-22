#include "ProgressIndicator.h"

#include <utility>

void ProgressIndicator::reportProgress(int current) {
    ENFORCE(std::this_thread::get_id() == outputThreadId);
    auto currentTime = getCurrentTimeMillis();
    if (enabled && (lastReported - currentTime + REPORTING_INTERVAL() <= std::chrono::seconds{0})) {
        lastReported = currentTime;
        progressbar_update(progress.get(), current);
    }
}

ProgressIndicator::~ProgressIndicator() {
    if (enabled) {
        progressbar_finish(progress.release());
    }
}

ProgressIndicator::ProgressIndicator(bool enabled, std::string name, int progressExpected)
    : progressExpected(progressExpected), name(std::move(name)), enabled(enabled) {
    outputThreadId = std::this_thread::get_id();
    if (enabled) {
        progress.reset(progressbar_new(this->name.c_str(), this->progressExpected));
        lastReported = getCurrentTimeMillis();
    }
}
