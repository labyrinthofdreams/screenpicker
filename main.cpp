#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <stdexcept>
#include "mainwindow.h"
#include "init.h"

void logToFile(QtMsgType msgType, const QMessageLogContext& ctx, const QString& msg)
{
    QString type;
    switch (msgType) {
    case QtDebugMsg:
        type = "Debug";
        break;
    case QtWarningMsg:
        type = "Warning";
        break;
    case QtCriticalMsg:
        type = "Critical";
        break;
    case QtFatalMsg:
        type = "Fatal";
        break;
    }

    const QString text = QString("[%1] (%2): %3").arg(ctx.category).arg(type).arg(msg);

    QFile outFile("log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);

    QTextStream ts(&outFile);
    ts << text << endl;
}

int main(int argc, char *argv[])
{
    // Create config
    QSettings config("config.ini", QSettings::IniFormat);
    vfg::config::merge(config, vfg::config::getDefaultSettings());

    if(config.value("enable_logging", false).toBool()) {
        qInstallMessageHandler(logToFile);
    }

    // Write scripts
    QDir current;
    current.mkdir("scripts");

    QFile::copy(":/scripts/imagemagick.ini", "scripts/imagemagick.ini");
    QFile::copy(":/scripts/gifsicle.ini", "scripts/gifsicle.ini");
    QFile::copy(":/scripts/x264.ini", "scripts/x264.ini");
    QFile::copy(":/scripts/d2v_template.avs", "scripts/d2v_template.avs");
    QFile::copy(":/scripts/default_template.avs", "scripts/default_template.avs");

    const QString appName = "ScreenPicker 3.0 (beta) rev1";
    QApplication a(argc, argv);
    a.setApplicationName(appName);
    a.setApplicationDisplayName(appName);
    a.setApplicationVersion(appName);

    try {
        MainWindow w;
        w.show();

        return a.exec();
    }
    catch(const std::exception& ex) {
        QMessageBox::critical(0, a.tr("Critical error"),
                              QString(ex.what()));
        a.exit(1);
    }

    return 1;
}
