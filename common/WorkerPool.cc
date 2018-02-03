#include "WorkerPool.h"

WorkerPool::WorkerPool(int size) : size(size) {
    for (int i = 0; i < size; i++) {
        threadQueues.emplace_back(std::make_unique<Queue>());
        auto &last = threadQueues.back();
        auto *ptr = last.get();
        threads.emplace_back(runInAThread([ptr]() {
            while (true) {
                Task task;
                ptr->wait_dequeue(task);
                task();
            }
        }));
    }
}

WorkerPool::~WorkerPool() {
    multiplexJob([]() { pthread_exit(nullptr); });
    // join will be called when destructing joinable;
}

void WorkerPool::multiplexJob(WorkerPool::Task t) {
    for (int i = 0; i < size; i++) {
        threadQueues[i]->enqueue(t);
    }
}
