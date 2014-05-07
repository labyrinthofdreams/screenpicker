#include <utility>
#include <QString>
#include "avisynthscriptparser.h"

namespace vfg {

AvisynthScriptParser::AvisynthScriptParser(QString scriptPath) :
    vfg::ScriptParser(std::move(scriptPath))
{
    setTemplate(path);
}

QString AvisynthScriptParser::parse(const QMap<QString, int>& settings) const
{
    Q_UNUSED(settings);

    return readTemplate(path);
}

} // namespace vfg
