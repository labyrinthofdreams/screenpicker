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

    condCopy(":/scripts/imagemagick.ini", "scripts/imagemagick.ini");
    condCopy(":/scripts/gifsicle.ini", "scripts/gifsicle.ini");
    condCopy(":/scripts/x264.ini", "scripts/x264.ini");
    condCopy(":/scripts/d2v_template.avs", "scripts/d2v_template.avs");
    condCopy(":/scripts/default_template.avs", "scripts/default_template.avs");

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
