#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <stdexcept>
#include "mainwindow.h"
#include "init.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ScreenPicker 2.0");
    a.setApplicationDisplayName("ScreenPicker 2.0");
    a.setApplicationVersion(QString("ScreenPicker 2.0 (beta) %1").arg(__TIMESTAMP__));

    try
    {
        // Create config
        QSettings config("config.ini", QSettings::IniFormat);
        vfg::config::merge(config, vfg::config::getDefaultSettings());

        // Write scripts        
        QDir current;
        current.mkdir("scripts");

        if(!QFile::exists("scripts/imagemagick.ini")) {
            QFile::copy(":/scripts/imagemagick.ini", "scripts/imagemagick.ini");
        }
        if(!QFile::exists("scripts/gifsicle.ini")) {
            QFile::copy(":/scripts/gifsicle.ini", "scripts/gifsicle.ini");
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
