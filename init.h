#ifndef STARTUP_H
#define STARTUP_H

#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QSettings>
#include <QMetaType>
#include <QPair>
#include <QImage>
#include <QStringList>

namespace vfg {
namespace init {
namespace meta {

void registerTypes()
{
    qRegisterMetaType<QPair<unsigned, QImage> >("QPair<unsigned, QImage>");
}

} // namespace meta

namespace script {

void createAvisynthTemplate()
{
    QDir appDir(QDir::currentPath());
    QString avisynthScriptFile = appDir.absoluteFilePath("default.avs");

    // Only create the default Avisynth script template file
    // if it doesn't exist to allow the user to change it
    if(QFile::exists(avisynthScriptFile))
        return;

    QFile inFile(":/scripts/default_template.avs");
    QFile outFile(avisynthScriptFile);

    if(!inFile.open(QFile::ReadOnly | QFile::Text))
        return;

    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
        return;

    QTextStream in(&inFile);
    QTextStream out(&outFile);
    out << in.readAll();
}

} // namespace script

namespace config {

bool isValid()
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    QStringList keys;
    keys << "avisynthpluginspath" << "savescripts" << "showscripteditor"
         << "maxthumbnails" << "numscreenshots" << "framestep"
         << "pauseafterlimit" << "removeoldestafterlimit"
         << "jumptolastonfinish" << "jumptolastonpause"
         << "jumptolastonstop" << "dgindexexecpath";
    QStringListIterator iter(keys);
    while(iter.hasNext())
    {
        const QString key = iter.next();
        if(!cfg.contains(key)) {
            return false;
        }
    }

    return true;
}

void create()
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    cfg.setValue("avisynthpluginspath", QDir::currentPath().append("/avisynth"));
    cfg.setValue("dgindexexecpath", "");
    cfg.setValue("savescripts", false);
    cfg.setValue("showscripteditor", true);
    cfg.setValue("maxthumbnails", 100);
    cfg.setValue("numscreenshots", 100);
    cfg.setValue("framestep", 100);
    cfg.setValue("pauseafterlimit", true);
    cfg.setValue("removeoldestafterlimit", false);
    cfg.setValue("jumptolastonfinish", true);
    cfg.setValue("jumptolastonpause", true);
    cfg.setValue("jumptolastonstop", true);
}

} // namespace config
} // namespace init
} // namespace vfg

#endif // STARTUP_H
