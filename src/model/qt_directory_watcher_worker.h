#ifndef QT_DIRECTORY_WATCHER_WORKER_H
#define QT_DIRECTORY_WATCHER_WORKER_H

#include "directory_watcher_worker.h"
#include <QObject>

class QtDirectoryWatcherWorker : public QObject, public DirectoryWatcherWorker
{
    Q_OBJECT

public:
    ~QtDirectoryWatcherWorker() override = default;

signals:
    void onStartWatching();
    void onStopWatching(std::exception_ptr exception);
    void onChangesArrived(DirectoryWatcher::ChangeIterator, DirectoryWatcher::ChangeIterator);

protected:
    void onStart() override
    {
        emit onStartWatching();
    }

    void onStop(std::exception_ptr exception) override
    {
        emit onStopWatching(exception);
    }

    void onUpdate(DirectoryWatcher::ChangeIterator begin, DirectoryWatcher::ChangeIterator end) override
    {
        emit onChangesArrived(begin, end);
    }
};

#endif // QT_DIRECTORY_WATCHER_WORKER_H
