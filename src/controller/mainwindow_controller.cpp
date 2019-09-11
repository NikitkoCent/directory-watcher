#include "../view/mainwindow.h"
#include "../model/file_operations.h"
#include "../model/path.h"
#include "../view/ui_mainwindow.h"
#include <stdexcept>        // std::exception
#include <utility>          // std::move
#include <QFileDialog>
#include <QString>
#include <QApplication>
#include <QMessageBox>
#include <QSignalBlocker>

MainWindowController::MainWindowController(MainWindow &parent)
    : QObject(&parent), parent(&parent)
{
}


void MainWindowController::changeTrackedPath()
{
    try
    {
        const QString name = QFileDialog::getExistingDirectory(parent, tr("Choose directory"));
        if (name.isEmpty())
        {
            return;
        }

        filesystem::Path path(name.toStdWString().c_str());
        parent->worker->run(std::move(path));
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(parent, tr("Error!"), err.what());
    }
    catch (...)
    {
        QMessageBox::critical(parent, tr("Error!"), tr("Unknown error"));
    }
}


void MainWindowController::renameFile(int row, int column)
{
    if (column != MainWindow::fileNameColumn)
    {
        return;
    }

    auto * const item = parent->ui->filesTable->item(row, column);
    const QString from = item->data(Qt::UserRole).toString();
    const QString to = item->text();

    if (from == to)
    {
        return;
    }

    try
    {
        const auto dir = parent->worker->getPath();
        const auto oldPath = dir / filesystem::Path(from.toStdWString().c_str());
        const auto newPath = dir / filesystem::Path(to.toStdWString().c_str());

        QSignalBlocker blocker(parent->ui->filesTable);
        filesystem::rename(oldPath, newPath);
        item->setData(Qt::UserRole, to);

        return;
    }
    catch (const std::exception &err)
    {
        QMessageBox::critical(parent, tr("Error!"), err.what());
    }
    catch (...)
    {
        QMessageBox::critical(parent, tr("Error!"), tr("Unknown error"));
    }

    QSignalBlocker blocker(parent->ui->filesTable);
    item->setText(from);
}


void MainWindowController::exit()
{
    QApplication::exit();
}
