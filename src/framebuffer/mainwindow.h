#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QListWidgetItem>

#include "imagebuffer.h"
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include "imageio.h"
#include "imagebuf.h"

OIIO_NAMESPACE_USING;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setupOCIO();

private slots:
    void on_actionPreferences_triggered();

    void on_action_Open_triggered();

    void updateHistoryList();

    void on_EvSlider_valueChanged(int value);

    void on_YSlider_valueChanged(int value);

    void on_actionIsolateR_triggered();

    void on_actionIsolateG_triggered();

    void on_actionIsolateB_triggered();

    void on_actionIsolateA_triggered();

    void on_actionIsolateRGBA_triggered();

    void on_EvButton_clicked();

    void on_YButton_clicked();

    void on_historyList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::MainWindow *ui;
    OCIO::ConstConfigRcPtr m_ocio_config;
    QColor  m_clearColor;
    QSettings m_settings;

    std::vector<ImageBuffer> ImagesBuffers;
};

#endif // MAINWINDOW_H
