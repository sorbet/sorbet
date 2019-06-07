#include "common/concurrency/WorkerPoolImpl.h"
#include "absl/strings/str_cat.h"
#include "common/concurrency/WorkerPool.h"

using namespace std;
namespace sorbet {
unique_ptr<WorkerPool> WorkerPool::create(int size, spd::logger &logger, bool pinThreads) {
    return make_unique<WorkerPoolImpl>(size, logger, pinThreads);
}

WorkerPool::~WorkerPool() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}

WorkerPoolImpl::WorkerPoolImpl(int size, spd::logger &logger, bool pinThreads) : size(size), logger(logger) {
    logger.debug("Creating {} worker threads", size);
    if (sorbet::emscripten_build) {
        ENFORCE(size == 0);
        this->size = 0;
    } else {
        threadQueues.reserve(size);
        for (int i = 0; i < size; i++) {
            auto &last = threadQueues.emplace_back(make_unique<Queue>());
            auto *ptr = last.get();
            auto threadIdleName = absl::StrCat("idle", i);
            optional<int> pinToCore;
            if (pinThreads) {
                pinToCore = i;
            }
            threads.emplace_back(runInAThread(
                threadIdleName,
                [ptr, &logger, threadIdleName]() {
                    bool repeat = true;
                    while (repeat) {
                        Task_ task;
                        setCurrentThreadName(threadIdleName);
                        ptr->wait_dequeue(task);
                        logger.debug("Worker got task");
                        repeat = task();
                    }
                },
                pinToCore));
        }
    }
    logger.debug("Worker threads created");
}

WorkerPoolImpl::~WorkerPoolImpl() {
    auto &logger = this->logger;
    multiplexJob_([&logger]() {
        logger.debug("Killing worker thread");
        return false;
    });
    // join will be called when destructing joinable;
}

void WorkerPoolImpl::multiplexJob(string_view taskName, WorkerPool::Task t) {
    if (size > 0) {
        multiplexJob_([t{move(t)}, taskName] {
            setCurrentThreadName(taskName);
            t();
            return true;
        });
    } else {
        // main thread is the worker.
        t();
    }
}

void WorkerPoolImpl::multiplexJob_(WorkerPoolImpl::Task_ t) {
    logger.debug("Multiplexing job");
    for (int i = 0; i < size; i++) {
        threadQueues[i]->enqueue(t);
    }
}

}; // namespace sorbet
