#include "NMainWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    NMainWidget w;
    w.show();

    return a.exec();
}
