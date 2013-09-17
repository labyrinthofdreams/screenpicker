#include <QString>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QMap>
#include <string>
#include <stdexcept>
#include "scriptparser.h"
#include "cpptempl.h"

namespace vfg {

inline std::wstring to_wstr(const int n)
{
    return QString::number(n).toStdWString();
}

ScriptParser::ScriptParser(QString scriptPath) :
    path(scriptPath),
    tplPath(":/scripts/default_template.avs")
{
}

ScriptParser::~ScriptParser()
{
}

QString ScriptParser::readTemplate(QString path)
{
    QFile tpl(path);
    if(!tpl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw ScriptParserTemplateException(QString("Unable to open %1").arg(path).toStdString());
    }

    QTextStream stream(&tpl);
    QString script = stream.readAll();
    return script;
}

void ScriptParser::setTemplate(QString path)
{
    tplPath = path;
}

QString ScriptParser::parse(QMap<QString, int> settings)
{
    try
    {
        using cpptempl::data_map;
        using cpptempl::make_data;
        QSettings cfg("config.ini", QSettings::IniFormat);
        QString script = readTemplate(tplPath);
        std::wstring str = script.toStdWString();
        data_map data;
        data[L"source_path"] = make_data(path.toStdWString());
        data[L"avs_plugins"] = make_data(cfg.value("avisynthpluginspath").toString().toStdWString());
        if(settings.value("ivtc", 0))
            data[L"ivtc"] = make_data(L"true");
        else
            data[L"ivtc"] = make_data(L"");

        if(settings.value("deinterlace", 0))
            data[L"deinterlace"] = make_data(L"true");
        else
            data[L"deinterlace"] = make_data(L"");

        // lambda function to convert odd numbers to even values
        // TODO: Probably doesn't belong here (unexpected value modification)
        auto upconvert_odd = [](const int n){return (n % 2 == 0) ? n : n + 1;};

        const int resizeWidth = settings.value("resizewidth", 0);
        const int resizeHeight = settings.value("resizeheight", 0);
        if(resizeWidth || resizeHeight) {
            data_map resize;
            resize[L"width"] = make_data(to_wstr(upconvert_odd(resizeWidth)));
            resize[L"height"] = make_data(to_wstr(upconvert_odd(resizeHeight)));
            data[L"resize"] = make_data(resize);
        }
        else {
            data[L"resize"] = make_data(L"");
        }

        const int cropTop = settings.value("croptop", 0);
        const int cropRight = settings.value("cropright", 0);
        const int cropBottom = settings.value("cropbottom", 0);
        const int cropLeft = settings.value("cropleft", 0);
        if(cropTop || cropRight || cropBottom || cropLeft) {
            data_map crop;
            crop[L"top"] = make_data(to_wstr(upconvert_odd(cropTop)));
            crop[L"right"] = make_data(to_wstr(upconvert_odd(cropRight)));
            crop[L"bottom"] = make_data(to_wstr(upconvert_odd(cropBottom)));
            crop[L"left"] = make_data(to_wstr(upconvert_odd(cropLeft)));
            data[L"crop"] = make_data(crop);
        }
        else {
            data[L"crop"] = make_data(L"");
        }

        std::wstring result = cpptempl::parse(str, data);

        return QString::fromStdWString(result);
    }
    catch(ScriptParserTemplateException& ex)
    {
        throw;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
