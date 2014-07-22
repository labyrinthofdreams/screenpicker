#ifndef INIT_H
#define INIT_H

#include <QDebug>
#include <QDir>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QVariant>

namespace vfg {
namespace config {

QMap<QString, QVariant> getDefaultSettings()
{
    QMap<QString, QVariant> cfg;
    cfg["avisynthpluginspath"] = QDir::currentPath().append("/avisynth");
    cfg["dgindexexecpath"] = "";
    cfg["showscripteditor"] = false;
    cfg["maxthumbnails"] = 1000;
    cfg["numscreenshots"] = 100;
    cfg["framestep"] = 100;
    cfg["pauseafterlimit"] = true;
    cfg["removeoldestafterlimit"] = false;
    cfg["jumptolastonfinish"] = true;
    cfg["jumptolastonpause"] = true;
    cfg["jumptolastonstop"] = true;
    cfg["jumptolastonreachingmax"] = true;
    cfg["savedgindexfiles"] = false;
    cfg["showvideosettings"] = false;
    cfg["resumegeneratorafterclear"] = false;
    cfg["gifsiclepath"] = QDir::currentPath().append("/gifsicle.exe");
    return cfg;
}

void merge(QSettings& cfg, const QMap<QString, QVariant>& defaults)
{
    // Note: for(e : range) selects a wrong iterator that only returns values
    for(auto kv = defaults.cbegin(), end = defaults.cend(); kv != end; ++kv) {
        if(!cfg.contains(kv.key())) {
            qDebug() << "Writing:" << kv.key();
            cfg.setValue(kv.key(), kv.value());
        }
    }
}

} // namespace config
} // namespace vfg

#endif // INIT_H
