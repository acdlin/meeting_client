#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<msgType>("msgType");
    QFont font("Microsoft YaHei", 8);
    QApplication::setFont(font);
    Widget w;
    w.show();
    return a.exec();
}
