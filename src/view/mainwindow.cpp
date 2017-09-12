#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "model/directory_watcher.h"
#include "model/file_operations.h"
#include <type_traits>              // std::remove_pointer, std::remove_reference
#include <chrono>                   // std::chrono::system_clock
#include <ctime>                    // std::time_t
#include <stdexcept>                // std::exception
#include <QMessageBox>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QString>
#include <QSignalBlocker>
#include <QDebug>

namespace
{
    using ChangeIter = DirectoryWatcher::ChangeIterator;
    using Change = DirectoryWatcher::ChangeEntry::ChangeType;

    template<typename T>
    class ObjectType
    {
    private:
        using rp = typename std::remove_pointer<T>::type;

    public:
        using type = typename std::remove_reference<rp>::type;
    };    


    void changeFilename(QTableWidget *filesTable, ChangeIter changeIter, const filesystem::Path &)
    {
        QString fileName = QString::fromWCharArray(changeIter->getCurrentPath().getPathString().c_str());

        QSignalBlocker blocker(filesTable);
        filesTable->item(changeIter->getFileIndex(), MainWindow::fileNameColumn)->setData(Qt::UserRole, fileName);
        filesTable->item(changeIter->getFileIndex(), MainWindow::fileNameColumn)->setText(fileName);
    }

    filesystem::FileType changeFiletype(QTableWidget *filesTable, ChangeIter changeIter, const filesystem::Path &fullPath)
    {        
        const auto result = filesystem::getFileType(fullPath);
        switch (result)
        {
            case filesystem::FileType::file:
                filesTable->item(changeIter->getFileIndex(), MainWindow::fileTypeColumn)->setText(QObject::tr("file"));
                break;
            case filesystem::FileType::directory:
                filesTable->item(changeIter->getFileIndex(), MainWindow::fileTypeColumn)->setText(QObject::tr("directory"));
                break;
            default:
                filesTable->item(changeIter->getFileIndex(), MainWindow::fileTypeColumn)->setText(QObject::tr("unknown"));
        }

        return result;
    }

    void changeModifyDate(QTableWidget *filesTable, ChangeIter changeIter, const filesystem::Path &fullPath)
    {        
        static const QString dateFormat = "dd-MM-yyyy hh:mm:ss";

        QDateTime date = QDateTime::fromTime_t(std::chrono::system_clock::to_time_t(filesystem::getModifyDate(fullPath)));
        filesTable->item(changeIter->getFileIndex(), MainWindow::modifyColumn)->setText(date.toString(dateFormat));
    }

    void changeFileSize(QTableWidget *filesTable, ChangeIter changeIter, const filesystem::Path &fullPath, filesystem::FileType fileType)
    {        
        if (fileType == filesystem::FileType::directory)
        {
            filesTable->item(changeIter->getFileIndex(), MainWindow::sizeColumn)->setText(QString('-'));
            return;
        }

        const auto size = filesystem::getFileSize(fullPath);
        filesTable->item(changeIter->getFileIndex(), MainWindow::sizeColumn)->setText(QString::number(size / 1024.0, 'f', 3));
    }

    void insertRow(QTableWidget *filesTable, ChangeIter changeIter, const filesystem::Path &fullPath)
    {
        filesTable->insertRow(changeIter->getFileIndex());

        filesTable->setItem(changeIter->getFileIndex(), 0, new QTableWidgetItem());
        for (int column = 1; column < 4; ++column)
        {
            auto * const item = new QTableWidgetItem();
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);

            filesTable->setItem(changeIter->getFileIndex(), column, item);
        }

        changeFilename(filesTable, changeIter, fullPath);
        changeFileSize(filesTable, changeIter, fullPath, changeFiletype(filesTable, changeIter, fullPath));
        changeModifyDate(filesTable, changeIter, fullPath);
    }
}


const QString MainWindow::trackedDirPrefix = QObject::tr("Tracked directory : ");


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),    
    ui(new Ui::MainWindow),
    controller(new MainWindowController(*this)),
    worker(new QtDirectoryWatcherWorker())
{
    ui->setupUi(this);

    ui->filesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    using ActionChangeType = typename ObjectType<decltype(ui->actionChange_directory)>::type;
    connect(ui->actionChange_directory, &ActionChangeType::triggered,
            controller, &MainWindowController::changeTrackedPath);

    using TableType = typename ObjectType<decltype(ui->filesTable)>::type;
    connect(ui->filesTable, &TableType::cellChanged, controller, &MainWindowController::renameFile);

    connect(worker, &QtDirectoryWatcherWorker::onStartWatching,
            this, &MainWindow::onStartWatching, Qt::BlockingQueuedConnection);

    connect(worker, &QtDirectoryWatcherWorker::onChangesArrived,
            this, &MainWindow::onChangesArrived, Qt::BlockingQueuedConnection);

    connect(worker, &QtDirectoryWatcherWorker::onStopWatching,
            this, &MainWindow::onStartWatching, Qt::BlockingQueuedConnection);

    using ActionExitType = typename ObjectType<decltype(ui->actionExit)>::type;
    connect(ui->actionExit, &ActionExitType::triggered, controller, &MainWindowController::exit);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onStartWatching()
{
	ui->filesTable->setRowCount(0);
    ui->trackedDirLabel->setText(trackedDirPrefix + QString::fromWCharArray(worker->getPath().getPathString().c_str()));
}

void MainWindow::onStopWatching(std::exception_ptr exception)
{
	try
    {
        if (exception)
        {
            std::rethrow_exception(exception);
        }
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(this, tr("Error!"), err.what());
    }
    catch (...)
    {
        QMessageBox::critical(this, tr("Error!"), tr("Unknown error"));
    }
}

void MainWindow::onChangesArrived(DirectoryWatcher::ChangeIterator begin, DirectoryWatcher::ChangeIterator end)
{    
    static const QString dateFormat = "dd-MM-yyyy hh:mm:ss";

    auto *const table = ui->filesTable;

    for (; begin != end; ++begin)
    {
        try
        {
            const auto path = worker->getPath() / begin->getCurrentPath();

            switch (begin->getType())
            {
                case Change::add:
                {
                    insertRow(table, begin, path);
                    break;
                }
                case Change::remove:                    
                    table->removeRow(begin->getFileIndex());
                    break;
                case Change::rename:
                {
                    QSignalBlocker blocker(table);
                    table->item(begin->getFileIndex(), fileNameColumn)->setData(Qt::UserRole, QString());
                    table->item(begin->getFileIndex(), fileNameColumn)->setText(QString());
                    changeFilename(table, begin, path);
                    break;
                }
                case Change::modify:
                {
                    table->item(begin->getFileIndex(), sizeColumn)->setText(QString());
                    table->item(begin->getFileIndex(), modifyColumn)->setText(QString());

                    changeFileSize(table, begin, path, filesystem::getFileType(path));
                    changeModifyDate(table, begin, path);

                    break;
                }
            }
        }
        catch (const std::exception &err)
        {
            qDebug() << err.what();
        }
        catch (...)
        {
            qDebug() << tr("Unknown error.");
        }
    }
}
