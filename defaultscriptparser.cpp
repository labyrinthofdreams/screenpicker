#include <QString>
#include <QSettings>
#include "defaultscriptparser.h"

namespace vfg {

DefaultScriptParser::DefaultScriptParser(QString path) :
    vfg::ScriptParser(path),
    scriptPath(path)
{
}

DefaultScriptParser::~DefaultScriptParser()
{
}

QString DefaultScriptParser::parse()
{
    try
    {
        QString script = readTemplate(":/scripts/default_template.avs");

        QSettings cfg("config.ini", QSettings::IniFormat);
        script.replace("[SOURCE_PATH]", scriptPath);
        script.replace("[AVISYNTH_PLUGINS]", cfg.value("avisynthpluginspath").toString());

        return script;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
