#include <QString>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <stdexcept>
#include "scriptparser.h"
#include "cpptempl.h"

namespace vfg {

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
        throw std::runtime_error("Script template error. File missing? Insufficient permissions?");
    }

    QTextStream stream(&tpl);
    QString script = stream.readAll();
    return script;
}

void ScriptParser::setTemplate(QString path)
{
    tplPath = path;
}

QString ScriptParser::parse()
{
    try
    {
        QSettings cfg("config.ini", QSettings::IniFormat);
        QString script = readTemplate(tplPath);
        std::wstring str = script.toStdWString();
        cpptempl::data_map data;
        data[L"SOURCE_PATH"] = cpptempl::make_data(path.toStdWString());
        data[L"AVS_PLUGINS"] = cpptempl::make_data(cfg.value("avisynthpluginspath").toString().toStdWString());
        data[L"IVTC"] = cpptempl::make_data(L"");
        data[L"deinterlace"] = cpptempl::make_data(L"");
        data[L"resize"] = cpptempl::make_data(L"");
        data[L"crop"] = cpptempl::make_data(L"");

        std::wstring result = cpptempl::parse(str, data);

        return QString::fromStdWString(result);
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
