#include "view/mainwindow.h"
#include <QApplication>
#include <thread>
#include <iostream>
#include <iomanip>
#include "model/directory_watcher.h"
#include "model/path.h"
#include "model/file_operations.h"
#include <chrono>
#include <ctime>
#include <io.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
