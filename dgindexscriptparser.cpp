#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <stdexcept>
#include "dgindexscriptparser.h"

namespace vfg {

DgindexScriptParser::DgindexScriptParser(QString path) :
    vfg::ScriptParser(path),
    scriptPath(path)
{
}

DgindexScriptParser::~DgindexScriptParser()
{
}

QString DgindexScriptParser::parse()
{
    try
    {
        QString script = readTemplate(":/scripts/d2v_template.avs");

        script.replace("[SOURCE_PATH]", scriptPath);

        return script;
    }
    catch(std::exception &ex)
    {
        throw;
    }
}

} // namespace vfg
