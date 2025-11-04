#include "mainwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/image/img/icon.jpg"));
    MainWidget w;
    w.show();
    return a.exec();
}
