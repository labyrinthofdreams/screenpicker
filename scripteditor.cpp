#include <stdexcept>
#include <QtGui>
#include "scripteditor.h"
#include "ui_scripteditor.h"

using namespace vfg;

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor),
    filepath()
{
    ui->setupUi(this);        
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

bool ScriptEditor::loadFile(QString path)
{
    QFile scriptfile(path);
    if(!scriptfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    ui->plainTextEdit->clear();
    QTextStream in(&scriptfile);
    while(!in.atEnd())
    {
        ui->plainTextEdit->appendPlainText(in.readLine());
    }

    filepath = path;

    return true;
}

void ScriptEditor::on_updateButton_clicked()
{
    // Write updated script
    QFile outFile(filepath);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Avisynth Script Editor"),
                              tr("Failed to write Avisynth script to disk. Make sure app directory is writable."));
        return;
    }

    QTextStream out(&outFile);
    out << ui->plainTextEdit->toPlainText();

    outFile.close();

    emit scriptUpdated(filepath);
}
