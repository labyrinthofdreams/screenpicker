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

/**
 * @brief Common DVD resolutions
 */
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
