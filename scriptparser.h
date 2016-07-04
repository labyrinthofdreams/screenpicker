#ifndef VFG_SCRIPTPARSER_H
#define VFG_SCRIPTPARSER_H

#include <stdexcept>
#include <string>
#include <QString>
#include <QtContainerFwd>

class QVariant;

namespace vfg {

struct ScriptParserError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class ScriptParser
{
public:
    explicit ScriptParser(const QString &filePath);

    QString parse(const QMap<QString, QVariant>& settings) const;

    void setTemplate(const QString &path);

protected:
    const QString path;
    QString tplPath {":/scripts/default_template.avs"};
};

} // namespace vfg

#endif // VFG_SCRIPTPARSER_H
