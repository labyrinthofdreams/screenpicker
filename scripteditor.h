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

        bool loadFile(QString path);
    private slots:
        void on_updateButton_clicked();

    private:
        Ui::ScriptEditor *ui;

        QString filepath;

    signals:
        void scriptUpdated(QString path);
    };
}

#endif // SCRIPTEDITOR_H
