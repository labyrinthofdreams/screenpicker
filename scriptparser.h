#ifndef VFG_SCRIPTPARSER_H
#define VFG_SCRIPTPARSER_H

class QString;

namespace vfg {

class ScriptParser
{
public:
    explicit ScriptParser(QString scriptPath);
    virtual ~ScriptParser();
    virtual QString parse();

protected:
    QString readTemplate(QString path);
    void setTemplate(QString path);

    QString path;
    QString tplPath;
};

} // namespace vfg

#endif // VFG_SCRIPTPARSER_H
