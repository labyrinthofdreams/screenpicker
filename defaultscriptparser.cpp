#include <utility>
#include <QString>
#include "defaultscriptparser.h"

namespace vfg {

DefaultScriptParser::DefaultScriptParser(QString path) :
    vfg::ScriptParser(std::move(path))
{
    setTemplate(":/scripts/default_template.avs");
}

} // namespace vfg
