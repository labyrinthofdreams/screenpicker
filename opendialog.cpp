#include <memory>
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

namespace vfg {
namespace ui {

OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);
}

void OpenDialog::setActiveTab(const OpenDialog::Tab tabName)
{
    if(tabName == Tab::OpenDisc) {
        ui.tabWidget->setCurrentIndex(0);
    }
    else if(tabName == Tab::OpenStream) {
        ui.tabWidget->setCurrentIndex(1);
    }
}

void OpenDialog::on_networkUrl_textEdited(const QString &arg1)
{
    // TODO: Display loading GIF
    static const vfg::extractor::ExtractorFactory factory;

    extractor = factory.getExtractor(arg1);
    connect(extractor.get(),    &vfg::extractor::BaseExtractor::streamsReady,
            this,               &OpenDialog::streamsReady);
    connect(extractor.get(),    &vfg::extractor::BaseExtractor::requestReady,
            this,               &OpenDialog::openUrl);
    connect(extractor.get(),    &vfg::extractor::BaseExtractor::logReady,
            ui.log,             &QPlainTextEdit::appendPlainText);
    extractor->fetchStreams(arg1);

    ui.streamsComboBox->clear();
    ui.log->clear();
}

void OpenDialog::streamsReady()
{
    // Display streams for the user
    ui.streamsComboBox->addItems(extractor->getStreams());
    ui.openButton->setEnabled(true);
}

void OpenDialog::on_openButton_clicked()
{
    if(ui.tabWidget->currentIndex() == 0) {
        // Process the selected DVD/BR files
        QStringList files;
        for(int i = 0; i < ui.fileList->topLevelItemCount(); ++i) {
            files.append(ui.fileList->topLevelItem(i)->text(0));
        }

        emit processDiscFiles(files);
    }
    else if(ui.tabWidget->currentIndex() == 1) {
        // Open the selected stream
        extractor->download(ui.streamsComboBox->currentText());
    }

    close();
}

void OpenDialog::on_cancelButton_clicked()
{
    close();
}

void OpenDialog::on_browseFiles_clicked()
{
    const auto files = QFileDialog::getOpenFileNames(this, tr("Select DVD VOB/Blu-ray M2TS files"),
                                                     config.value("last_opened_dvd", "").toString(),
                                                     "DVD VOB (*.vob);;Blu-ray M2TS (*.m2ts)");
    if(files.isEmpty()) {
        return;
    }

    QList<QTreeWidgetItem*> items;
    for(const auto &file : files) {
        items.append(new QTreeWidgetItem(QStringList() << file));
    }

    ui.fileList->insertTopLevelItems(0, items);
    ui.fileList->sortItems(0, Qt::AscendingOrder);
    ui.openButton->setEnabled(true);
}

void OpenDialog::on_clearButton_clicked()
{
    ui.fileList->clear();
    ui.openButton->setEnabled(false);
}

void OpenDialog::on_removeButton_clicked()
{
    const std::unique_ptr<QTreeWidgetItem> discard {ui.fileList->takeTopLevelItem(
                    ui.fileList->currentIndex().row())};
}

} // namespace ui
} // namespace vfg
