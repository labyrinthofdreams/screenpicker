#include <stdexcept>
#include <QtWidgets>
#include <QDir>
#include <QFileDialog>
#include "scripteditor.h"

namespace vfg {
namespace ui {

QString defaultPath() {
    static const auto path = QDir::current().absoluteFilePath("temp_script.avs");
    return path;
}

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);

    setSavePath(defaultPath());
}

ScriptEditor::~ScriptEditor()
{
    if(QFile::exists(defaultPath())) {
        QFile::remove(defaultPath());
    }
}

void ScriptEditor::save()
{
    // Write updated script
    QFile outFile(savePath);
    if(!outFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QMessageBox::critical(this, tr("Avisynth Script Editor"),
                              tr("Failed to open Avisynth script for writing. "
                                 "Make sure folder isn't read-only."));
        return;
    }

    QTextStream out(&outFile);
    out << ui.plainTextEdit->toPlainText();
}

void ScriptEditor::setContent(const QString &content)
{
    ui.plainTextEdit->setPlainText(content);
}

QString ScriptEditor::path() const
{
    return savePath;
}

void ScriptEditor::setSavePath(const QString &path)
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
    const QString outPath = QFileDialog::getSaveFileName(0,
                                                         tr("Select Avisynth script output path"),
                                                         defaultPath());
    if(outPath.isEmpty()) {
        return;
    }

    setSavePath(outPath);
    save();
}

void ScriptEditor::reset()
{
    ui.plainTextEdit->clear();
    setSavePath(defaultPath());
}

} // namespace ui
} // namespace vfg
