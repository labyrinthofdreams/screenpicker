#include <utility>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QMap>
#include <string>
#include "scriptparser.h"
#include "templet.hpp"

vfg::ScriptParser::ScriptParser(QString filePath) :
    path(std::move(filePath)),
    tplPath(":/scripts/default_template.avs")
{
}

QString vfg::ScriptParser::readTemplate(const QString& path) const
{
    QFile tpl(path);
    if(!tpl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw vfg::ScriptParserError(QString("Unable to open %1").arg(path).toStdString());
    }

    QTextStream stream(&tpl);
    return stream.readAll();
}

void vfg::ScriptParser::setTemplate(QString path)
{
    tplPath = std::move(path);
}

QString vfg::ScriptParser::parse(const QMap<QString, int>& settings) const
{
    try
    {
        using templet::make_data;
        QSettings cfg("config.ini", QSettings::IniFormat);
        QString script = readTemplate(tplPath);
        templet::DataMap data;
        data["source_path"] = make_data(path.toStdString());
        data["avs_plugins"] = make_data(cfg.value("avisynthpluginspath").toString().toStdString());

        if(settings.value("ivtc", 0)) {
            data["ivtc"] = make_data("true");
        }

        if(settings.value("deinterlace", 0)) {
            data["deinterlace"] = make_data("true");
        }

        const int resizeWidth = settings.value("resizewidth", 0);
        const int resizeHeight = settings.value("resizeheight", 0);
        if(resizeWidth || resizeHeight) {
            templet::DataMap resize;
            resize["width"] = make_data(resizeWidth);
            resize["height"] = make_data(resizeHeight);
            data["resize"] = make_data(resize);
        }

        const int cropTop = settings.value("croptop", 0);
        const int cropRight = settings.value("cropright", 0);
        const int cropBottom = settings.value("cropbottom", 0);
        const int cropLeft = settings.value("cropleft", 0);
        if(cropTop || cropRight || cropBottom || cropLeft) {
            templet::DataMap crop;
            crop["top"] = make_data(cropTop);
            crop["right"] = make_data(cropRight);
            crop["bottom"] = make_data(cropBottom);
            crop["left"] = make_data(cropLeft);
            data["crop"] = make_data(crop);
        }

        templet::Templet tpl(script.toStdString());

        return QString::fromStdString(tpl.parse(data));
    }
    catch(vfg::ScriptParserError& ex)
    {
        throw;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}
