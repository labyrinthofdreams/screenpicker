#include <stdexcept>
#include <QtWidgets>
#include <QDir>
#include <QFileDialog>
#include "scripteditor.h"
#include "ui_scripteditor.h"

// TODO: QWidget window flag to force create a Window, then construct with new ScriptEditor(this)

namespace vfg {

QString ScriptEditor::defaultPath()
{
    static QString path = QDir::current().absoluteFilePath("temp_script.avs");
    return path;
}

namespace ui {

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    setSavePath(defaultPath());
}

ScriptEditor::~ScriptEditor()
{
    if(QFile::exists(defaultPath())) {
        QFile::remove(defaultPath());
    }
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

QString ScriptEditor::path() const
{
    return savePath;
}

void ScriptEditor::setSavePath(QString path)
{
    savePath = path;
    setWindowTitle(path);
}

void ScriptEditor::on_updateButton_clicked()
{
    save();

    emit scriptUpdated();
}

void ScriptEditor::on_btnSaveAs_clicked()
{
    QString outPath = QFileDialog::getSaveFileName(0, tr("Select Avisynth script output path"),
                                                      defaultPath());
    if(outPath.isEmpty()) {
        return;
    }

    setSavePath(outPath);
    save();
}

void ScriptEditor::reset()
{
    ui->plainTextEdit->clear();
    setSavePath(defaultPath());
}

} // namespace ui
} // namespace vfg
