#include <QString>
#include "defaultscriptparser.h"

namespace vfg {

DefaultScriptParser::DefaultScriptParser(QString path) :
    vfg::ScriptParser(path)
{
    setTemplate(":/scripts/default_template.avs");
}

DefaultScriptParser::~DefaultScriptParser()
{
}

} // namespace vfg
