#ifndef VFG_DGINDEXSCRIPTPARSER_H
#define VFG_DGINDEXSCRIPTPARSER_H

#include "scriptparser.h"

class QString;

namespace vfg {

class DgindexScriptParser : public vfg::ScriptParser
{
public:
    explicit DgindexScriptParser(QString scriptPath);
    ~DgindexScriptParser();
};

} // namespace vfg

#endif // VFG_DGINDEXSCRIPTPARSER_H
