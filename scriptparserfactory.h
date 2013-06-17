#ifndef VFG_SCRIPTPARSERFACTORY_H
#define VFG_SCRIPTPARSERFACTORY_H

#include <QSharedPointer>

class QString;

namespace vfg {
    class ScriptParser;
}

namespace vfg {

class ScriptParserFactory
{
public:
    ScriptParserFactory();
    QSharedPointer<ScriptParser> parser(QString script);
};

} // namespace vfg

#endif // VFG_SCRIPTPARSERFACTORY_H
