#ifndef VFG_VIDEOSETTINGSWIDGET_H
#define VFG_VIDEOSETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QString>

// Forward declarations
class QCloseEvent;
class QShowEvent;
class QRect;

namespace Ui {
    class VideoSettingsWidget;
}

namespace vfg {

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

namespace ui {

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

    void handleCropChange();

    void on_btnRevertCrop_clicked();

private:
    Ui::VideoSettingsWidget *ui;

    QMap<QString, int> prevSettings;

    struct {
        int left, top, right, bottom;
    } crop = {0, 0, 0, 0};

protected:
    void showEvent(QShowEvent *event);
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

} // namespace ui
} // namespace vfg
#endif // VFG_VIDEOSETTINGSWIDGET_H
