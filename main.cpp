#include <QtGui/QApplication>
#include <QMessageBox>
#include <QString>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try
    {
        MainWindow w;
        w.show();

        return a.exec();
    }
    catch(std::exception& ex)
    {
        QMessageBox::critical(0, "Critical error",
                              QString(ex.what()));
        a.exit(1);
    }

    return 1;
}
