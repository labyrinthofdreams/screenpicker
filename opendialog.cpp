#include <QComboBox>
#include <QNetworkRequest>
#include <QUrl>
#include "extractorfactory.hpp"
#include "opendialog.hpp"
#include "ui_opendialog.h"

vfg::ui::OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
}

vfg::ui::OpenDialog::~OpenDialog()
{
    delete ui;
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
    ui->streamsComboBox->clear();
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
    // Open the selected stream
    extractor->download(ui->streamsComboBox->currentText());

    close();
}

void vfg::ui::OpenDialog::on_cancelButton_clicked()
{
    close();
}
