#include <stdexcept>
#include <QtGui>
#include "scripteditor.h"
#include "ui_scripteditor.h"

// TODO: QWidget window flag to force create a Window, then construct with new ScriptEditor(this)

using namespace vfg;

QString script::parse(QString filepath)
{
    QFile scriptfile("default.avs");
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error("Failed to open script");
    }

    QTextStream in(&scriptfile);

    // Load config
    QSettings cfg("config.ini", QSettings::IniFormat);
    QString pluginsPath = cfg.value("avisynthpluginspath").toString();

    // Parse avisynth script
    QString parsedScript = in.readAll();
    parsedScript = parsedScript.arg(filepath).arg(pluginsPath);

    return parsedScript;
}

void script::save(QString path, QString script)
{
    QFile outFile(path);
    if(!outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        throw std::runtime_error("Failed to open file for writing.");
    }

    QTextStream out(&outFile);
    out << script;
}

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

void ScriptEditor::setContent(QString path)
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

    outFile.close();

    emit scriptUpdated(savePath);
}
