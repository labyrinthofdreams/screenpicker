#include <QString>
#include <QFileInfo>
#include <QSharedPointer>
#include "scriptparserfactory.h"
#include "scriptparser.h"
#include "dgindexscriptparser.h"
#include "avisynthscriptparser.h"
#include "defaultscriptparser.h"

namespace vfg {

ScriptParserFactory::ScriptParserFactory()
{
}

QSharedPointer<vfg::ScriptParser> ScriptParserFactory::parser(QString script)
{
    vfg::ScriptParser *scriptParser;
    QFileInfo info(script);
    QString suffix = info.suffix();
    if(suffix == "avs" || suffix == "avsi")
        scriptParser = new vfg::AvisynthScriptParser(script);
    else if(suffix == "d2v")
        scriptParser = new vfg::DgindexScriptParser(script);
    else
        scriptParser = new vfg::ScriptParser(script);

    return QSharedPointer<vfg::ScriptParser>(scriptParser);
}

} // namespace vfg
