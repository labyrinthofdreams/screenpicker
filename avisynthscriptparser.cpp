#include <QString>
#include "avisynthscriptparser.h"

namespace vfg {

AvisynthScriptParser::AvisynthScriptParser(QString path) :
    vfg::ScriptParser(path),
    scriptPath(path)
{
}

AvisynthScriptParser::~AvisynthScriptParser()
{
}

QString AvisynthScriptParser::parse()
{
    try
    {
        QString script = readTemplate(scriptPath);

        return script;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
