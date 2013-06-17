#include <QString>
#include <QFileInfo>
#include <QSharedPointer>
#include "scriptparserfactory.h"
#include "scriptparser.h"

namespace vfg {

ScriptParserFactory::ScriptParserFactory()
{
}

QSharedPointer<vfg::ScriptParser> ScriptParserFactory::parser(QString script)
{
    vfg::ScriptParser *scriptParser;
    QFileInfo info(script);
    QString suffix = info.suffix();
    switch(suffix)
    {
    case "avs":
    case "avsi":
        scriptParser = new vfg::AvisynthScriptParser(script);
        break;
    case "d2v":
        scriptParser = new vfg::DgindexScriptParser(script);
        break;
    default:
        scriptParser = new vfg::DefaultScriptParser(script);
        break;
    }

    return QSharedPointer<vfg::ScriptParser>(scriptParser);
}

} // namespace vfg
