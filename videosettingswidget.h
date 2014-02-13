#ifndef VFG_VIDEOSETTINGSWIDGET_H
#define VFG_VIDEOSETTINGSWIDGET_H

#include <QMap>
#include <QString>
#include <QWidget>

// Forward declarations
class QCloseEvent;
class QRect;
class QShowEvent;

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
     * @brief Destructor
     **/
    ~VideoSettingsWidget();

    /**
     * @brief Retrieve current settings
     * @return Settings as a map
     */
    QMap<QString, int> getSettings() const;

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
    void handleCropChange();

    /**
     * @brief Revert previously applied crop
     */
    void on_btnRevertCrop_clicked();

private:
    Ui::VideoSettingsWidget *ui;

    QMap<QString, int> prevSettings;

    struct {
        int left, top, right, bottom;
    } crop = {0, 0, 0, 0};

    /**
     * @brief Common DVD aspect ratios for the resize dropdown
     */
    enum AspectRatio : int {
        Default_AR = 0, //!< Default aspect ratio
        NTSC_16_9 = 1, //!< NTSC 16:9
        NTSC_4_3 = 2, //!< NTSC 4:3
        PAL_16_9 = 3, //!< PAL 16:9
        PAL_4_3 = 4 //!< PAL 4:3
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
    void cropChanged(QRect area);

    /**
     * @brief Signal window close event
     */
    void closed();
};

} // namespace ui
} // namespace vfg

#endif // VFG_VIDEOSETTINGSWIDGET_H
