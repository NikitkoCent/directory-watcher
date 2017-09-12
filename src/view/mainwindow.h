#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model/qt_directory_watcher_worker.h"
#include "model/directory_watcher.h"
#include <QMainWindow>
#include <QObject>
#include <exception>        // std::exception_ptr

namespace Ui {
    class MainWindow;
}


class MainWindow;

class MainWindowController : public QObject
{
    Q_OBJECT

public slots:
    void renameFile(int row, int column);
    void changeTrackedPath();
    void exit();

private:
    friend class MainWindow;
    MainWindowController(MainWindow &parent);

    MainWindow *parent;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static constexpr auto fileNameColumn = 0;
    static constexpr auto fileTypeColumn = 1;
    static constexpr auto modifyColumn = 2;
    static constexpr auto sizeColumn = 3;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    

private slots:
    void onStartWatching();
    void onStopWatching(std::exception_ptr exception);
    void onChangesArrived(DirectoryWatcher::ChangeIterator, DirectoryWatcher::ChangeIterator);

private:       
    friend class MainWindowController;

    static const QString trackedDirPrefix;    
    Ui::MainWindow *ui;
    MainWindowController *const controller;
    QtDirectoryWatcherWorker *const worker;    
};

#endif // MAINWINDOW_H
