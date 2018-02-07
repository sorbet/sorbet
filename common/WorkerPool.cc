#include "WorkerPool.h"

WorkerPool::WorkerPool(int size, std::shared_ptr<spd::logger> logger) : size(size), logger(logger) {
    logger->trace("Creating {} worker threads", size);
    for (int i = 0; i < size; i++) {
        threadQueues.emplace_back(std::make_unique<Queue>());
        auto &last = threadQueues.back();
        auto *ptr = last.get();
        threads.emplace_back(runInAThread([ptr, logger]() {
            while (true) {
                Task task;
                ptr->wait_dequeue(task);
                logger->trace("Worker got task");
                task();
            }
        }));
    }
    logger->trace("Worker threads created");
}

WorkerPool::~WorkerPool() {
    auto logger = this->logger;
    multiplexJob([logger]() {
        logger->trace("Killing worker thread");
        pthread_exit(nullptr);
    });
    // join will be called when destructing joinable;
}

void WorkerPool::multiplexJob(WorkerPool::Task t) {
    logger->trace("Multiplexing job");
    for (int i = 0; i < size; i++) {
        threadQueues[i]->enqueue(t);
    }
}
