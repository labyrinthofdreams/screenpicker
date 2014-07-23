#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <stdexcept>
#include "mainwindow.h"
#include "init.h"

/**
 * @brief Conditionally copy file "from" to "to" if "to" doesn't exist
 * @param from Copy from path
 * @param to Copy to path
 * @return True on success, otherwise false
 */
bool condCopy(const QString& from, const QString& to) {
    if(QFile::exists(to)) {
        return false;
    }

    return QFile::copy(from, to);
}

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

        condCopy(":/scripts/imagemagick.ini", "scripts/imagemagick.ini");
        condCopy(":/scripts/gifsicle.ini", "scripts/gifsicle.ini");
        condCopy(":/scripts/d2v_template.avs", "scripts/d2v_template.avs");
        condCopy(":/scripts/default_template.avs", "scripts/default_template.avs");

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
