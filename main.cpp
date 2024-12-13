#include "mainwindow.h"
#include <time.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    srand(std::time(NULL));
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
