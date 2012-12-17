#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QString>

namespace Ui {
class ScriptEditor;
}

namespace vfg
{
    class ScriptEditor : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScriptEditor(QWidget *parent = 0);
        ~ScriptEditor();

        void loadVideo(QString path);
        void loadScript(QString path);
    private slots:
        void on_updateButton_clicked();

    private:
        Ui::ScriptEditor *ui;

    signals:
        // Emits name of the temporary script path
        // with our modified script
        void scriptUpdated(QString tmpScriptPath);
    };
}

#endif // SCRIPTEDITOR_H
