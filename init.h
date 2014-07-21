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
namespace config {

bool isValid()
{
    QSettings cfg("config.ini", QSettings::IniFormat);
    QStringList keys;
    keys << "avisynthpluginspath" << "showscripteditor"
         << "maxthumbnails" << "numscreenshots" << "framestep"
         << "pauseafterlimit" << "removeoldestafterlimit"
         << "jumptolastonfinish" << "jumptolastonpause"
         << "jumptolastonstop" << "dgindexexecpath" << "jumptolastonreachingmax"
         << "savedgindexfiles" << "showvideosettings"
         << "resumegeneratorafterclear";
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
    cfg.setValue("showscripteditor", false);
    cfg.setValue("maxthumbnails", 100);
    cfg.setValue("numscreenshots", 100);
    cfg.setValue("framestep", 100);
    cfg.setValue("pauseafterlimit", true);
    cfg.setValue("removeoldestafterlimit", false);
    cfg.setValue("jumptolastonfinish", true);
    cfg.setValue("jumptolastonpause", true);
    cfg.setValue("jumptolastonstop", true);
    cfg.setValue("jumptolastonreachingmax", true);
    cfg.setValue("savedgindexfiles", false);
    cfg.setValue("showvideosettings", false);
    cfg.setValue("resumegeneratorafterclear", false);
}

} // namespace config
} // namespace init
} // namespace vfg

#endif // STARTUP_H
