#ifndef VFG_VIDEOSETTINGSWIDGET_H
#define VFG_VIDEOSETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QString>

namespace vfg {

namespace Ui {
class VideoSettingsWidget;
}

enum class Resolution : int {
    OTHER = 0,
    NTSC_169 = 1,
    NTSC_43 = 2,
    PAL_169 = 3,
    PAL_43 = 4
};

enum class Deinterlace : int {
    NONE = 0,
    IVTC = 1,
    DEINTERLACE = 2
};

class VideoSettingsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit VideoSettingsWidget(QWidget *parent = 0);
    ~VideoSettingsWidget();

    QMap<QString, int> getSettings() const;
    void resetSettings();
    
private slots:
    void on_cboxDvdResolution_activated(int index);

    void on_pushButton_clicked();

private:
    Ui::VideoSettingsWidget *ui;

    void closeEvent(QCloseEvent *event);

signals:
    void settingsChanged();
    /**
     * @brief cropChanged emits new crop values
     * @param area Area to crop on the image
     */
    void cropChanged(QRect area);

    void closed();
};


} // namespace vfg
#endif // VFG_VIDEOSETTINGSWIDGET_H
