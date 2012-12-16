#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMap>

// Forward declarations
class QPoint;

namespace vfg {
    class VideoFrameGrabber;
    class VideoFrameThumbnail;
    class ScriptEditor;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:

    void on_actionOpen_triggered();

    void loadFromAvisynthScript(QString path);

    void videoLoaded();

    void videoError(QString msg);

    void thumbnailDoubleClicked(vfg::VideoFrameThumbnail* thumbnail);

    void handleUnsavedMenu(const QPoint& pos);

    void handleSavedMenu(const QPoint& pos);

    void on_nextButton_clicked();

    void on_previousButton_clicked();

    void on_generateButton_clicked();

    void on_originalResolutionCheckBox_toggled(bool checked);

    void on_seekSlider_valueChanged(int value);

    void on_seekSlider_sliderMoved(int position);

    void on_grabButton_clicked();

    void on_actionSave_thumbnails_triggered();

    void on_clearThumbsButton_clicked();

    void on_thumbnailSizeSlider_sliderMoved(int position);

    void on_thumbnailSizeSlider_valueChanged(int value);

    void on_saveThumbnailsButton_clicked();

    void on_actionAvisynth_Script_Editor_triggered();

private:
    Ui::MainWindow *ui;

    vfg::VideoFrameGrabber* frameGrabber;

    vfg::ScriptEditor* scriptEditor;

    QString avisynthScriptFile;

    QMap<int, QImage> unsaved;
    QMap<int, QImage> saved;

    void createAvisynthScriptFile();
};

#endif // MAINWINDOW_H
