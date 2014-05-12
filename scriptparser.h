#ifndef VFG_SCRIPTPARSER_H
#define VFG_SCRIPTPARSER_H

#include <stdexcept>
#include <string>
#include <QString>
#include <QtContainerFwd>

namespace vfg {

class ScriptParserError : public std::runtime_error
{
public:
    explicit ScriptParserError(const char* msg) : std::runtime_error(msg) {}
    explicit ScriptParserError(const std::string& msg) : std::runtime_error(msg) {}
};

class ScriptParser
{
public:
    explicit ScriptParser(QString filePath);
    virtual ~ScriptParser() = default;
    virtual QString parse(const QMap<QString, int>& settings) const;
    virtual void setTemplate(QString path);

protected:
    QString readTemplate(const QString& path) const;

    QString path;
    QString tplPath;
};

} // namespace vfg

#endif // VFG_SCRIPTPARSER_H
