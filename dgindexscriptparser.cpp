#include <utility>
#include <QString>
#include "dgindexscriptparser.h"

vfg::DgindexScriptParser::DgindexScriptParser(QString scriptPath) :
    vfg::ScriptParser(std::move(scriptPath))
{
    setTemplate(":/scripts/d2v_template.avs");
}
