#include <QString>
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include "scriptparser.h"

namespace vfg {

QString ScriptParser::readTemplate(QString path)
{
    QFile tpl(path);
    if(!tpl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Script template error. File missing? Insufficient permissions?");
    }

    QTextStream stream(&tpl);
    QString script = stream.readAll();
    return script;
}

} // namespace vfg
