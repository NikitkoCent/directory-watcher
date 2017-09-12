#ifndef DIRECTORY_WATCHER_WORKER_H
#define DIRECTORY_WATCHER_WORKER_H

#include "path.h"
#include "file_operations.h"
#include "directory_watcher.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <exception>

/* Abstract class DirectoryWatcherWorker represents a wrapper around DirectoryWatcher with a built-in separate thread for DirectoryWatcher running
 * DirectoryWatcherWorker isn't thread safe; intended for use in parent thread */
class DirectoryWatcherWorker
{
public:    
    virtual ~DirectoryWatcherWorker();

    /*
     * Calls stop() and then launch a new DirectoryWatcher in build-in thread
     *
     * Parameters:
     *  path    -   full path to tracked directory; will be passed to new DirectoryWatcher instance
     */
    void run(const filesystem::Path &path);
    void run(filesystem::Path &&path);

    /*
     * Interrupts current DirectoryWatcher and turns a built-in thread to sleeping state
     */
    void stop();

    /*
     * returns path to tracked at now directory
     * returns Path() if no directory processing
     */
    const filesystem::Path& getPath() const;

protected:
    DirectoryWatcherWorker();

    /* Calls when a new DirectoryWatcher starts tracking directory */
    virtual inline void onStart() {}

    /* Calls when a DirectoryWatcher was interrupted */
    virtual inline void onStop(std::exception_ptr) {}

    /*
     * Calls when directory change event(s) have been registered
     * Parameters
     *  begin, end = iterators of range [begin, end) of changes
     */
    virtual inline void onUpdate(DirectoryWatcher::ChangeIterator begin,
                                 DirectoryWatcher::ChangeIterator end)
    { (void)begin; (void)end; }

private:
    class WorkerRoutine;

    DirectoryWatcher *watcher = nullptr;
    filesystem::Path path;
    mutable std::mutex mutex;
    std::condition_variable workerSleep;
    std::thread workerThread;
    bool wakeUp = false, needExit = false;    


    DirectoryWatcherWorker(const DirectoryWatcherWorker &src) = delete;
    DirectoryWatcherWorker& operator=(const DirectoryWatcherWorker &right) = delete;
    DirectoryWatcherWorker(DirectoryWatcherWorker &&src) = delete;
    DirectoryWatcherWorker& operator=(DirectoryWatcherWorker &&right) = delete;

    template<typename Path>
    void runImpl(Path &&path)
    {
        std::lock_guard<decltype(mutex)> lock(mutex);

        stopWithoutLock();

        this->path = std::forward<Path>(path);
        wakeUp = true;
        workerSleep.notify_one();        
    }


    void stopWithoutLock();
};

#endif // DIRECTORY_WATCHER_WORKER_H
