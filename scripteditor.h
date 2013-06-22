#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>
#include <QString>

namespace Ui {
class ScriptEditor;
}

namespace vfg
{
    namespace script
    {
        void save(QString path, QString script);
    }

    class ScriptEditor : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScriptEditor(QWidget *parent = 0);
        ~ScriptEditor();

        // Loads contents of script file
        void setContent(QString path);
        QString getPath() const;
    private slots:
        void on_updateButton_clicked();

    private:
        Ui::ScriptEditor *ui;

        QString savePath;
    signals:
        // Emits name of the temporary script path
        // with our modified script
        void scriptUpdated();
    };
}

#endif // SCRIPTEDITOR_H
