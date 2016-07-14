#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QString>
#include "ui_scripteditor.h"

namespace Ui {
class ScriptEditor;
}

namespace vfg {
namespace ui {

class ScriptEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptEditor(QWidget *parent = 0);

    ~ScriptEditor();

    // Loads contents of script file
    void setContent(const QString &content);
    void save();
    void reset();
    QString path() const;

private slots:
    void on_updateButton_clicked();

    void on_btnSaveAs_clicked();

private:
    ::Ui::ScriptEditor ui;

    QString savePath;

    void setSavePath(const QString &path);

signals:
    void scriptUpdated();
};

} // namespace ui
} // namespace vfg

#endif // SCRIPTEDITOR_H
