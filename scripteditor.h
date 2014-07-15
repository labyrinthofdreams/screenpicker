#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QString>

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
    void setContent(QString content);
    void save();
    void reset();
    QString path() const;

    static QString defaultPath();
private slots:
    void on_updateButton_clicked();

    void on_btnSaveAs_clicked();

private:
    ::Ui::ScriptEditor *ui;

    QString savePath;
    void setSavePath(QString path);
signals:
    void scriptUpdated();
};

} // namespace ui
} // namespace vfg

#endif // SCRIPTEDITOR_H
