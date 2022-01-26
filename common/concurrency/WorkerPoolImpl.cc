#include "common/concurrency/WorkerPoolImpl.h"
#include "absl/strings/str_cat.h"
#include "common/concurrency/WorkerPool.h"

using namespace std;
namespace sorbet {
unique_ptr<WorkerPool> WorkerPool::create(int size, spd::logger &logger) {
    return make_unique<WorkerPoolImpl>(size, logger);
}

WorkerPool::~WorkerPool() {
    // see https://eli.thegreenplace.net/2010/11/13/pure-virtual-destructors-in-c
}

WorkerPoolImpl::WorkerPoolImpl(int size, spd::logger &logger) : _size(size), logger(logger) {
    logger.trace("Creating {} worker threads", _size);
    if (sorbet::emscripten_build) {
        ENFORCE(size == 0);
        this->_size = 0;
    } else {
        bool pinThreads = (_size > 0) && (_size == thread::hardware_concurrency());
        threadQueues.reserve(_size);
        for (int i = 0; i < _size; i++) {
            auto &last = threadQueues.emplace_back(make_unique<Queue>());
            auto *ptr = last.get();
            auto threadIdleName = absl::StrCat("idle", i + 1);
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
                        logger.trace("Worker got task");
                        repeat = task();
                    }
                },
                pinToCore));
        }
    }
    logger.trace("Worker threads created");
}

WorkerPoolImpl::~WorkerPoolImpl() {
    auto &logger = this->logger;
    multiplexJob_([&logger]() {
        logger.trace("Killing worker thread");
        return false;
    });
    // join will be called when destructing joinable;
}

void WorkerPoolImpl::multiplexJob(string_view taskName, WorkerPool::Task t) {
    if (_size > 0) {
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
    logger.trace("Multiplexing job");
    for (int i = 0; i < _size; i++) {
        const bool enqueued = threadQueues[i]->enqueue(t);
        ENFORCE(enqueued, "Failed to enqueue (did we surpass MAX_SUBQUEUE_SIZE items enqueued?)");
    }
}

int WorkerPoolImpl::size() {
    return _size;
}

}; // namespace sorbet
