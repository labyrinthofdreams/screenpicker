#include <QString>
#include <QSettings>
#include <stdexcept>
#include "dgindexscriptparser.h"

namespace vfg {

DgindexScriptParser::DgindexScriptParser(QString scriptPath) :
    vfg::ScriptParser(scriptPath)
{
    setTemplate(":/scripts/d2v_template.avs");
}

DgindexScriptParser::~DgindexScriptParser()
{
}

} // namespace vfg
