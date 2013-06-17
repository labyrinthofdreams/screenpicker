#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <stdexcept>
#include "dgindexscriptparser.h"

namespace vfg {

DgindexScriptParser::DgindexScriptParser(QString path) :
    vfg::ScriptParser(),
    scriptPath(path)
{
}

QString DgindexScriptParser::parse()
{
    QFile tpl(":/scripts/d2v_template.avs");
    if(!tpl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open script template.");
    }

    QTextStream stream(&tpl);
    QString script = stream.readAll();

    script.replace("[SOURCE_PATH]", scriptPath);

    return script;
}

} // namespace vfg
