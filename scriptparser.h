#ifndef VFG_SCRIPTPARSER_H
#define VFG_SCRIPTPARSER_H

#include <QMap>
class QString;

namespace vfg {

class ScriptParserTemplateException : public std::runtime_error
{
public:
    explicit ScriptParserTemplateException(const char* msg) : std::runtime_error(msg) {}
    explicit ScriptParserTemplateException(const std::string& msg) : std::runtime_error(msg) {}
};

class ScriptParser
{
public:
    explicit ScriptParser(QString scriptPath);
    virtual ~ScriptParser();
    virtual QString parse(QMap<QString, int> settings);

protected:
    QString readTemplate(QString path);
    void setTemplate(QString path);

    QString path;
    QString tplPath;
};

} // namespace vfg

#endif // VFG_SCRIPTPARSER_H
