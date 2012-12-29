#include <stdexcept>
#include <QtGui>
#include "scripteditor.h"
#include "ui_scripteditor.h"

using namespace vfg;

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor),
    savePath("temp.avs")
{
    ui->setupUi(this);        
}

ScriptEditor::~ScriptEditor()
{
    QFile::remove("temp.avs");
    delete ui;
}

void ScriptEditor::load(QString path)
{
    QFile scriptfile(path);
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open file for reading.");
    }

    QTextStream in(&scriptfile);
    ui->plainTextEdit->setPlainText(in.readAll());
}

void ScriptEditor::on_updateButton_clicked()
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

    emit scriptUpdated(savePath);
}
