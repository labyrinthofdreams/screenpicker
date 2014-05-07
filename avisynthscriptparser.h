#ifndef VFG_AVISYNTHSCRIPTPARSER_H
#define VFG_AVISYNTHSCRIPTPARSER_H

#include <QtContainerFwd>
#include "scriptparser.h"

class QString;

namespace vfg {

class AvisynthScriptParser : public vfg::ScriptParser
{
public:
    explicit AvisynthScriptParser(QString scriptPath);
    ~AvisynthScriptParser() = default;
    QString parse(const QMap<QString, int>& settings) const override;
};

} // namespace vfg

#endif // VFG_AVISYNTHSCRIPTPARSER_H
