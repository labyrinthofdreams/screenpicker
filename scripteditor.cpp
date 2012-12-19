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

void ScriptEditor::loadScript(QString path)
{       
    QFile scriptfile(path);
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open file for reading.");
    }

    setSavePath(path);

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

    setSavePath(path);

    QTextStream in(&inFile);
    QString parsedScript = in.readAll().arg(path).arg(QDir::currentPath());

    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText(parsedScript);

    on_updateButton_clicked();
}

void ScriptEditor::load(QString path)
{
    try
    {
        QFileInfo file(path);
        if(file.suffix() == "avs")
        {
            loadScript(path);
        }
        else
        {
            loadVideo(path);
        }
    }
    catch(std::exception& ex)
    {
        throw;
    }
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

    outFile.close();

    emit scriptUpdated(savePath);
}

void ScriptEditor::setSavePath(QString path)
{
    QFileInfo fi(path);
    savePath = tr("%1/%2.avs").arg(fi.dir().path()).arg(fi.completeBaseName());
}
