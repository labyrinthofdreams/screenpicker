#include <utility>
#include <QString>
#include "defaultscriptparser.h"

vfg::DefaultScriptParser::DefaultScriptParser(QString path) :
    vfg::ScriptParser(std::move(path))
{
    setTemplate(":/scripts/default_template.avs");
}
