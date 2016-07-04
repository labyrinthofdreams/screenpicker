#include <string>
#include <sstream>
#include <QFile>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QVariant>
#include "scriptparser.h"
#include "templet.hpp"

namespace {

QString readTemplate(const QString& path) {
    QFile tpl(path);
    if(!tpl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw vfg::ScriptParserError(QString("Unable to open %1").arg(path).toStdString());
    }

    QTextStream stream(&tpl);
    return stream.readAll();
}

} // namespace

namespace vfg {

ScriptParser::ScriptParser(const QString &filePath) :
    path(filePath)
{
}

void ScriptParser::setTemplate(const QString &path)
{
    tplPath = path;
}

QString ScriptParser::parse(const QMap<QString, QVariant>& settings) const try
{
    using templet::make_data;

    templet::DataMap data;
    data["source_path"] = make_data(path.toStdString());
    data["avs_plugins"] = make_data(settings.value("avisynthpluginspath")
                                    .toString().toStdString());

    if(settings.value("ivtc", 0).toInt()) {
        data["ivtc"] = make_data("true");
    }

    if(settings.value("deinterlace", 0).toInt()) {
        data["deinterlace"] = make_data("true");
    }

    const int resizeWidth = settings.value("resizewidth", 0).toInt();
    const int resizeHeight = settings.value("resizeheight", 0).toInt();
    if(resizeWidth || resizeHeight) {
        templet::DataMap resize;
        resize["width"] = make_data(resizeWidth);
        resize["height"] = make_data(resizeHeight);
        data["resize"] = make_data(resize);
    }

    const int cropTop = settings.value("croptop", 0).toInt();
    const int cropRight = settings.value("cropright", 0).toInt();
    const int cropBottom = settings.value("cropbottom", 0).toInt();
    const int cropLeft = settings.value("cropleft", 0).toInt();
    if(cropTop || cropRight || cropBottom || cropLeft) {
        templet::DataMap crop;
        crop["top"] = make_data(cropTop);
        crop["right"] = make_data(cropRight);
        crop["bottom"] = make_data(cropBottom);
        crop["left"] = make_data(cropLeft);
        data["crop"] = make_data(crop);
    }

    const std::string tpl = readTemplate(tplPath).toStdString();
    std::ostringstream oss;
    templet::parse(tpl, data, oss);

    return QString::fromStdString(oss.str());
}
catch(const vfg::ScriptParserError& ex)
{
    throw;
}
catch(const std::exception &ex)
{
    throw;
}

} // namespace vfg
