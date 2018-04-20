#include "WorkerPool.h"

WorkerPool::WorkerPool(int size, std::shared_ptr<spd::logger> logger) : size(size), logger(logger) {
    logger->debug("Creating {} worker threads", size);
    for (int i = 0; i < size; i++) {
        threadQueues.emplace_back(std::make_unique<Queue>());
        auto &last = threadQueues.back();
        auto *ptr = last.get();
        threads.emplace_back(runInAThread([ptr, logger]() {
            bool repeat = true;
            while (repeat) {
                Task_ task;
                ptr->wait_dequeue(task);
                logger->debug("Worker got task");
                repeat = task();
            }
        }));
    }
    logger->debug("Worker threads created");
}

WorkerPool::~WorkerPool() {
    auto logger = this->logger;
    multiplexJob_([logger]() {
        logger->debug("Killing worker thread");
        return false;
    });
    // join will be called when destructing joinable;
}

void WorkerPool::multiplexJob(WorkerPool::Task t) {
    multiplexJob_([t{move(t)}] {
        t();
        return true;
    });
}

void WorkerPool::multiplexJob_(WorkerPool::Task_ t) {
    logger->debug("Multiplexing job");
    for (int i = 0; i < size; i++) {
        threadQueues[i]->enqueue(t);
    }
}
