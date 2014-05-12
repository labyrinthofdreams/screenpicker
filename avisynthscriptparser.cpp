#include <utility>
#include <QString>
#include "avisynthscriptparser.h"

vfg::AvisynthScriptParser::AvisynthScriptParser(QString scriptPath) :
    vfg::ScriptParser(std::move(scriptPath))
{
    setTemplate(path);
}

QString vfg::AvisynthScriptParser::parse(const QMap<QString, int>& settings) const
{
    Q_UNUSED(settings);

    return readTemplate(path);
}
