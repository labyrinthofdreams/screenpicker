#ifndef VFG_SCRIPTPARSER_H
#define VFG_SCRIPTPARSER_H

class QString;

namespace vfg {

class ScriptParser
{
public:
    explicit ScriptParser(QString script) {}
    virtual ~ScriptParser() {}
    virtual QString parse() = 0;
};

} // namespace vfg

#endif // VFG_SCRIPTPARSER_H
