/**
 * File: thread-pool.cc
 * --------------------
 * Presents the implementation of the ThreadPool class.
 */

#include "thread-pool.h"
using namespace std;

thread_local int currentWorkerId = -1;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false), remainingTasks(0) {
    for (size_t i = 0; i < numThreads; i++) {
        wts[i].ts = thread([this, i]() { worker(i); });
    }
    dt = thread([this]() { dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (done) throw runtime_error("Cannot schedule on destroyed ThreadPool.");
    if (!thunk) throw runtime_error("Cannot schedule a null task.");

    {
        lock_guard<mutex> lg(globalLock);
        tasks.push(thunk);
        remainingTasks++;
    }
    taskAvailable.notify_all();
}

void ThreadPool::wait() {
    unique_lock<mutex> lk(globalLock);

    if (currentWorkerId != -1) {
        wts[currentWorkerId].waiting = true;
    }

    allDone.wait(lk, [this]() {
        return remainingTasks == 0;
    });

    if (currentWorkerId != -1) {
        wts[currentWorkerId].waiting = false;
    }

    if (firstException) {
        rethrow_exception(firstException);
    }
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> task;

        {
            unique_lock<mutex> lk(globalLock);
            taskAvailable.wait(lk, [this]() { return !tasks.empty() || done; });

            if (done && tasks.empty()) return;

            bool foundWorker = false;
            while (!foundWorker) {
                for (size_t i = 0; i < wts.size(); ++i) {
                    if (wts[i].available) {
                        task = tasks.front();
                        tasks.pop();
                        wts[i].thunk = task;
                        wts[i].available = false;
                        wts[i].ready.signal();
                        foundWorker = true;
                        break;
                    }
                }

                if (!foundWorker) {
                    taskAvailable.wait(lk);
                    if (done && tasks.empty()) return;
                }
            }
        }
    }
}

void ThreadPool::worker(int id) {
    currentWorkerId = id;

    while (true) {
        wts[id].ready.wait();

        {
            lock_guard<mutex> lg(globalLock);
            if (done) return;
        }

        try {
            wts[id].thunk();
        } catch (...) {
            lock_guard<mutex> lg(globalLock);
            if (!firstException) {
                firstException = current_exception();
            }
        }

        {
            lock_guard<mutex> lg(globalLock);
            wts[id].available = true;
            remainingTasks--;
            if (remainingTasks == 0) {
                allDone.notify_all();
            }
        }

        taskAvailable.notify_all();
    }
}

ThreadPool::~ThreadPool() {
    try {
        wait();
    } catch (...) {
        // Destructor debe ignorar excepciones propagadas
    }

    {
        lock_guard<mutex> lg(globalLock);
        done = true;
    }
    taskAvailable.notify_all();

    if (dt.joinable()) dt.join();

    for (auto& w : wts) {
        w.ready.signal();
        if (w.ts.joinable()) w.ts.join();
    }
}
