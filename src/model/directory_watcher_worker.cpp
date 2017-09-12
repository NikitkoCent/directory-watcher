#include "directory_watcher_worker.h"
#include <utility>  // std::move


class DirectoryWatcherWorker::WorkerRoutine
{
public:
    WorkerRoutine(DirectoryWatcherWorker &context)
        : context(context)
    {
    }

    void operator()() const
    {
        std::unique_lock<decltype(context.mutex)> lock(context.mutex);

        context.workerSleep.wait(lock, [this]{ return (context.wakeUp || context.needExit); });
        context.wakeUp = false;

        while (!context.needExit)
        {
            std::exception_ptr exception = nullptr;

            try
            {
                context.onStart();

                DirectoryWatcher watcher(context.path);
                context.watcher = &watcher;

                lock.unlock();

                using Iter = DirectoryWatcher::ChangeIterator;
                watcher.startWatch([this](Iter begin, Iter end){ context.onUpdate(begin, end); });

                lock.lock();
            }
            catch (...)
            {
                if (!lock)
                {
                    lock.lock();
                }

                exception = std::current_exception();
            }

            context.watcher = nullptr;

            context.onStop(exception);

            context.workerSleep.wait(lock, [this]{ return (context.wakeUp || context.needExit); });
            context.wakeUp = false;
        }
    }

private:
    DirectoryWatcherWorker &context;
};



DirectoryWatcherWorker::DirectoryWatcherWorker()
{
    workerThread = std::move(std::thread(WorkerRoutine(*this)));
}

DirectoryWatcherWorker::~DirectoryWatcherWorker()
{
    {
        std::lock_guard<decltype(mutex)> lock(mutex);

        stopWithoutLock();
        needExit = true;

        workerSleep.notify_one();
    }

    if (workerThread.joinable())
    {
        workerThread.join();
    }
}


void DirectoryWatcherWorker::run(const filesystem::Path &path)
{
    runImpl(path);
}

void DirectoryWatcherWorker::run(filesystem::Path &&path)
{
    runImpl(std::move(path));
}


void DirectoryWatcherWorker::stop()
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    stopWithoutLock();
}


const filesystem::Path& DirectoryWatcherWorker::getPath() const
{
    return path;
}


void DirectoryWatcherWorker::stopWithoutLock()
{
    if (watcher == nullptr)
    {
        return;
    }

    watcher->stopWatch();
}
