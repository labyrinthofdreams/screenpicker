#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QFile>
#include <stdexcept>
#include "mainwindow.h"
#include "init.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ScreenPicker 2.0");
    a.setApplicationDisplayName("ScreenPicker 2.0");
    a.setApplicationVersion("2.0 BETA r20130811.2037");

    try
    {
        {
            using namespace vfg::init;

            meta::registerTypes();

            if(QFile::exists("config.ini")) {
                if(!config::isValid()) {
                    QMessageBox::StandardButton clicked =
                            QMessageBox::question(0, a.tr("Configuration error"),
                                                  a.tr("Outdated configuration file. Do you want to remove old settings?"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes);
                    if(clicked == QMessageBox::Yes) {
                        QFile::remove("config.ini");
                        config::create();
                    }
                    else {
                        throw std::runtime_error("Can't proceed with outdated configuration file");
                    }
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
        QMessageBox::critical(0, a.tr("Critical error"),
                              QString(ex.what()));
        a.exit(1);
    }

    return 1;
}
