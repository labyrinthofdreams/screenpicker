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

        // Load video using the default Avisynth script template
        void loadVideo(QString path);

        // Load Avisynth script
        void loadScript(QString path);

        // Generalized method that calls either loadVideo()
        // or loadScript() depending on the file suffix
        void load(QString path);
    private slots:
        void on_updateButton_clicked();

    private:
        Ui::ScriptEditor *ui;

        QString savePath;

        void setSavePath(QString path);

    signals:
        // Emits name of the temporary script path
        // with our modified script
        void scriptUpdated(QString tmpScriptPath);
    };
}

#endif // SCRIPTEDITOR_H
