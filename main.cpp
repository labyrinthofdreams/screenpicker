#include <QtGui/QApplication>
#include <QMessageBox>
#include <QString>
#include <QFile>
#include <stdexcept>
#include "mainwindow.h"
#include "init.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try
    {
        {
            using namespace vfg::init;

            meta::registerTypes();

            script::createAvisynthTemplate();

            if(QFile::exists("config.ini")) {
                if(!config::isValid()) {
                    throw std::runtime_error("Invalid configuration file. Please remove config.ini and restart");
                }
            }
            else {
                config::create();
            }
        }

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
