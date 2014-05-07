#ifndef VFG_DEFAULTSCRIPTPARSER_H
#define VFG_DEFAULTSCRIPTPARSER_H

#include "scriptparser.h"

class QString;

namespace vfg {

class DefaultScriptParser : public vfg::ScriptParser
{
public:
    explicit DefaultScriptParser(QString path);
    ~DefaultScriptParser();
};

} // namespace vfg

#endif // VFG_DEFAULTSCRIPTPARSER_H
