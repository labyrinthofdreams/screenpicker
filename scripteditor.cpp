#include <stdexcept>
#include <QtWidgets>
#include <QDir>
#include "scripteditor.h"
#include "ui_scripteditor.h"

// TODO: QWidget window flag to force create a Window, then construct with new ScriptEditor(this)

namespace vfg {

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor),
    savePath()
{
    ui->setupUi(this);

    QDir localDir(QDir::currentPath());
    setPath(localDir.absoluteFilePath("temp_script.avs"));
}

ScriptEditor::~ScriptEditor()
{
    // TODO: Remove only local temp file in same dir
    QFile::remove("temp_script.avs");
    delete ui;
}

void ScriptEditor::save()
{
    // Write updated script
    QFile outFile(savePath);
    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::critical(this, tr("Avisynth Script Editor"),
                              tr("Failed to open Avisynth script for writing. Make sure folder isn't read-only."));
        return;
    }

    QTextStream out(&outFile);
    out << ui->plainTextEdit->toPlainText();
}

void ScriptEditor::setContent(QString content)
{
    ui->plainTextEdit->setPlainText(content);
}

void ScriptEditor::setPath(QString path)
{
    savePath = path;
}

QString ScriptEditor::getPath() const
{
    return savePath;
}

void ScriptEditor::on_updateButton_clicked()
{
    save();

    emit scriptUpdated();
}

} // namespace vfg
