#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QNetworkRequest>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QUrl>
#include "common.hpp"
#include "extractorfactory.hpp"
#include "opendialog.hpp"
#include "ui_opendialog.h"

vfg::ui::OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog),
    config("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
}

vfg::ui::OpenDialog::~OpenDialog()
{
    delete ui;
}

void vfg::ui::OpenDialog::setActiveTab(vfg::ui::OpenDialog::Tab tabName)
{
    if(tabName == Tab::OpenDisc) {
        ui->tabWidget->setCurrentIndex(0);
    }
    else if(tabName == Tab::OpenStream) {
        ui->tabWidget->setCurrentIndex(1);
    }
}

void vfg::ui::OpenDialog::on_networkUrl_textEdited(const QString &arg1)
{
    // TODO: Display loading GIF
    static const vfg::extractor::ExtractorFactory factory;
    extractor = factory.getExtractor(arg1);
    connect(extractor.get(), SIGNAL(streamsReady()),
            this, SLOT(streamsReady()));
    connect(extractor.get(), SIGNAL(requestReady(QNetworkRequest)),
            this, SIGNAL(openUrl(QNetworkRequest)));
    connect(extractor.get(), SIGNAL(logReady(QString)),
            ui->log, SLOT(appendPlainText(QString)));
    ui->streamsComboBox->clear();
    ui->log->clear();
    extractor->fetchStreams(arg1);
}

void vfg::ui::OpenDialog::streamsReady()
{
    // Display streams for the user
    ui->streamsComboBox->addItems(extractor->getStreams());
    ui->openButton->setEnabled(true);
}

void vfg::ui::OpenDialog::on_openButton_clicked()
{
    if(ui->tabWidget->currentIndex() == 0) {
        // Process the selected DVD/BR files
        QStringList files;
        for(int i = 0; i < ui->fileList->topLevelItemCount(); ++i) {
            files.append(ui->fileList->topLevelItem(i)->text(0));
        }

        emit processDiscFiles(files);
    }
    else if(ui->tabWidget->currentIndex() == 1) {
        // Open the selected stream
        extractor->download(ui->streamsComboBox->currentText());
    }

    close();
}

void vfg::ui::OpenDialog::on_cancelButton_clicked()
{
    close();
}

void vfg::ui::OpenDialog::on_browseFiles_clicked()
{
    const QStringList files = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                         config.value("last_opened_dvd", "").toString(),
                                                         "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(files.empty()) {
        return;
    }

    QList<QTreeWidgetItem*> items;
    for(const QString &file : files) {
        const QFileInfo fi(file);
        items.append(new QTreeWidgetItem(QStringList() << file << vfg::format::formatNumber(fi.size())));
    }

    ui->fileList->insertTopLevelItems(0, items);
    ui->fileList->sortItems(0, Qt::AscendingOrder);
    ui->openButton->setEnabled(true);
}

void vfg::ui::OpenDialog::on_clearButton_clicked()
{
    ui->fileList->clear();
    ui->openButton->setEnabled(false);
}
