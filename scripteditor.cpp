#include <stdexcept>
#include <QtGui>
#include "scripteditor.h"
#include "ui_scripteditor.h"

using namespace vfg;

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);        
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

void ScriptEditor::loadScript(QString path)
{
    QFile scriptfile(path);
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open file for reading.");
    }

    QTextStream in(&scriptfile);
    QString script = in.readAll();

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(script);

    on_updateButton_clicked();
}

void ScriptEditor::loadVideo(QString path)
{
    QFile inFile("default.avs");
    if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open script file for reading.");
    }

    QTextStream in(&inFile);
    QString parsedScript = in.readAll().arg(path);

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(parsedScript);

    on_updateButton_clicked();
}

void ScriptEditor::on_updateButton_clicked()
{
    // Write updated script
    QFile outFile("temp.avs");
    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::critical(this, tr("Avisynth Script Editor"),
                              tr("Failed to open Avisynth script for writing. Make sure folder isn't read-only."));
        return;
    }

    QTextStream out(&outFile);
    out << ui->plainTextEdit->toPlainText();

    outFile.close();

    emit scriptUpdated("temp.avs");
}
