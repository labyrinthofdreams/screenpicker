#include <QString>
#include "avisynthscriptparser.h"

namespace vfg {

AvisynthScriptParser::AvisynthScriptParser(QString scriptPath) :
    vfg::ScriptParser(scriptPath)
{
    setTemplate(scriptPath);
}

AvisynthScriptParser::~AvisynthScriptParser()
{
}

QString AvisynthScriptParser::parse()
{
    try
    {
        QString script = readTemplate(path);

        return script;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
