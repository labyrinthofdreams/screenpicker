#ifndef VFG_AVISYNTHSCRIPTPARSER_H
#define VFG_AVISYNTHSCRIPTPARSER_H

#include "scriptparser.h"

class QString;

namespace vfg {

class AvisynthScriptParser : public vfg::ScriptParser
{
public:
    explicit AvisynthScriptParser(QString scriptPath);
    ~AvisynthScriptParser();
    QString parse();
};

} // namespace vfg

#endif // VFG_AVISYNTHSCRIPTPARSER_H
