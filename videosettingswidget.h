#ifndef VFG_VIDEOSETTINGSWIDGET_H
#define VFG_VIDEOSETTINGSWIDGET_H

#include <memory>
#include <QMap>
#include <QSettings>
#include <QWidget>
#include "ui_videosettingswidget.h"

class QCloseEvent;
class QRect;
class QShowEvent;
class QString;
class QVariant;

namespace Ui {
    class VideoSettingsWidget;
}

namespace vfg {
namespace ui {

/**
 * @brief The VideoSettingsWidget class
 *
 * The video settings widget contains UI elements
 * for modifying the input video
 */
class VideoSettingsWidget : public QWidget
{
    Q_OBJECT   
public:
    /**
     * @brief Constructor
     * @param parent Owner of the widget
     */
    explicit VideoSettingsWidget(QWidget *parent = 0);

    /**
     * @brief Retrieve current settings
     * @return Settings as a map
     */
    QMap<QString, QVariant> getSettings() const;

    /**
     * @brief Reset all settings to default values
     */
    void resetSettings();
    
private slots:
    /**
     * @brief Handle DVD resolution dropdown change
     *
     * Sets the resolution in the input boxes when a DVD resolution
     * is selected from the dropdown menu
     * @param index
     */
    void on_cboxDvdResolution_activated(int index);

    /**
     * @brief Apply changes and emit \link settingsChanged() \endlink
     */
    void on_pushButton_clicked();

    /**
     * @brief Emit \link cropChanged() \endlink with area to crop
     */
    void handleCropChange(int);

    /**
     * @brief Revert previously applied crop
     */
    void on_btnRevertCrop_clicked();

    void on_radioLockWidth_clicked();

    void on_radioLockHeight_clicked();

    void on_radioLockDefault_clicked();

    void on_sboxResizeWidth_valueChanged(int arg1);

    void on_sboxResizeHeight_valueChanged(int arg1);

private:
    ::Ui::VideoSettingsWidget ui;

    QSettings config {"config.ini", QSettings::IniFormat};

    QMap<QString, QVariant> prevSettings {};

    struct CropArea {
        int left {0};
        int top {0};
        int right {0};
        int bottom {0};
    };
    CropArea crop {};

    //! Video width used when calculating new resolution
    int videoWidth {0};

    //! Video height used when calculating new resolution
    int videoHeight {0};

    //! Original source video width
    int sourceWidth {0};

    //! Original source video height
    int sourceHeight {0};

    /**
     * @brief Common DVD aspect ratios for the resize dropdown
     */
    enum AspectRatio : int {
        Original = 0,    //!< Original resolution
        NTSC_16_9,  //!< NTSC 16:9
        NTSC_4_3,   //!< NTSC 4:3
        PAL_16_9,   //!< PAL 16:9
        PAL_4_3,    //!< PAL 4:3
    };

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

signals:
    /**
     * @brief Signal changed settings
     */
    void settingsChanged();

    /**
     * @brief Signal changed crop values
     * @param area Area to crop
     */
    void cropChanged(const QRect& area);

    /**
     * @brief Signal window close event
     */
    void closed();
};

} // namespace ui
} // namespace vfg

#endif // VFG_VIDEOSETTINGSWIDGET_H
