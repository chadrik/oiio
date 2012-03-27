#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "preferencesdialog.h"
#include <QtGui/QFileDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setupOCIO();
    ui->setupUi(this);
    ui->inSpaceCombo->clear();
    ui->outSpaceCombo->clear();

    for(int s=0; s<m_ocio_config->getNumColorSpaces(); ++s)
    {
        ui->inSpaceCombo->addItem(m_ocio_config->getColorSpaceNameByIndex(s));
        std::cout << "Combo Space: " << m_ocio_config->getColorSpaceNameByIndex(s) << std::endl;
    }

    for(int i=0; i<m_ocio_config->getNumViews( m_ocio_config->getDefaultDisplay()); ++i)
    {
        ui->outSpaceCombo->addItem(m_ocio_config->getView(m_ocio_config->getDefaultDisplay(), i));
        std::cout << "View: " << m_ocio_config->getView(m_ocio_config->getDefaultDisplay(), i) << std::endl;
    }
    m_clearColor = Qt::green;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupOCIO()
{
    int errorcount = 0;
    std::cout << "OpenColorIO Library Version: " << OCIO::GetVersion() << std::endl;
    std::cout << "OpenColorIO Library VersionHex: " << OCIO::GetVersionHex() << std::endl;
    std::string inputconfig = "/madcrew/applications/SDK/ocio/imageworks-OpenColorIO-Configs-fc29cd9/nuke-default/config.ocio";
    std::cout << "Loading " << inputconfig << std::endl;
    m_ocio_config = OCIO::Config::CreateFromFile(inputconfig.c_str());
    std::cout << "Default Display: " << m_ocio_config->getDefaultDisplay() << std::endl;
    std::cout << "Default View: " << m_ocio_config->getDefaultView(m_ocio_config->getDefaultDisplay()) << std::endl;
    std::cout << "** ColorSpaces **" << std::endl;
    for(int i=0; i<m_ocio_config->getNumColorSpaces(); ++i)
    {
            std::cout << "Space: " << m_ocio_config->getColorSpaceNameByIndex(i) << std::endl;
    }
    std::cout << "** Looks **" << std::endl;
    if(m_ocio_config->getNumLooks()>0)
    {
        for(int i=0; i<m_ocio_config->getNumLooks(); ++i)
        {
            std::cout << m_ocio_config->getLookNameByIndex(i) << std::endl;
        }
    }
    else
    {
        std::cout << "no looks defined" << std::endl;
    }
    try
    {
        m_ocio_config->sanityCheck();
        std::cout << "passed" << std::endl;
    }
    catch(OCIO::Exception & exception)
    {
        std::cout << "ERROR" << std::endl;
        errorcount += 1;
        std::cout << exception.what() << std::endl;
    }
    for(int i=0; i<m_ocio_config->getNumDisplays(); ++i)
    {
        std::cout << "Display: " << m_ocio_config->getDisplay(i) << std::endl;
    }
    OCIO::SetCurrentConfig(m_ocio_config);
}

void MainWindow::on_actionPreferences_triggered()
{
    PreferencesDialog *prefdialog = new PreferencesDialog(this);
    prefdialog->show();
}

void MainWindow::on_action_Open_triggered()
{
    QString path = m_settings.value("file/recentFile", "").toString();
    std::cout << "Settings: " << path.toStdString().c_str() << std::endl;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path, tr("Images (*.exr *.hdr *.jpg)"));
    QFileInfo dir(fileName);
    m_settings.setValue("file/recentFile", dir.dir().path());
    std::cout << "File " << dir.dir().path().toStdString().c_str() << std::endl;
    ImageBuffer img = ImageBuffer(fileName.toStdString());
    ImagesBuffers.push_back(img);
    updateHistoryList();
}

void MainWindow::updateHistoryList(){
    std::vector<ImageBuffer>::iterator end = ImagesBuffers.end();
    ui->historyList->clear();
    for (std::vector<ImageBuffer>::iterator it = ImagesBuffers.begin(); it < end; ++it) {
        ui->historyList->addItem(it->getName().c_str());
        std::cout << ": " << it->getName().c_str() << std::endl;
        //std::cout << " Score(value = second): " << it->second << '\n';
    }
}

void MainWindow::on_EvSlider_valueChanged(int value)
{
    ui->BufferWindow->exposure_CB((float)value/10.0f);
    if(value != 0){
        ui->EvButton->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 255, 255)");
    }else{
        ui->EvButton->setStyleSheet("");
    }
}

void MainWindow::on_YSlider_valueChanged(int value)
{
    ui->BufferWindow->gamma_CB((float)value/100.0f);
    if(value != 100){
        ui->YButton->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 255, 255)");
    }else{
        ui->YButton->setStyleSheet("");
    }
}

void MainWindow::on_actionIsolateR_triggered()
{
    ui->BufferWindow->m_channelHot[0] = 1;
    ui->BufferWindow->m_channelHot[1] = 0;
    ui->BufferWindow->m_channelHot[2] = 0;
    ui->BufferWindow->m_channelHot[3] = 0;
    ui->BufferWindow->UpdateOCIOGLState();
    ui->BufferWindow->updateGL();
}

void MainWindow::on_actionIsolateG_triggered()
{
    ui->BufferWindow->m_channelHot[0] = 0;
    ui->BufferWindow->m_channelHot[1] = 1;
    ui->BufferWindow->m_channelHot[2] = 0;
    ui->BufferWindow->m_channelHot[3] = 0;
    ui->BufferWindow->UpdateOCIOGLState();
    ui->BufferWindow->updateGL();
}

void MainWindow::on_actionIsolateB_triggered()
{
    ui->BufferWindow->m_channelHot[0] = 0;
    ui->BufferWindow->m_channelHot[1] = 0;
    ui->BufferWindow->m_channelHot[2] = 1;
    ui->BufferWindow->m_channelHot[3] = 0;
    ui->BufferWindow->UpdateOCIOGLState();
    ui->BufferWindow->updateGL();
}

void MainWindow::on_actionIsolateA_triggered()
{
    ui->BufferWindow->m_channelHot[0] = 0;
    ui->BufferWindow->m_channelHot[1] = 0;
    ui->BufferWindow->m_channelHot[2] = 0;
    ui->BufferWindow->m_channelHot[3] = 1;
    ui->BufferWindow->UpdateOCIOGLState();
    ui->BufferWindow->updateGL();
}

void MainWindow::on_actionIsolateRGBA_triggered()
{
    ui->BufferWindow->m_channelHot[0] = 1;
    ui->BufferWindow->m_channelHot[1] = 1;
    ui->BufferWindow->m_channelHot[2] = 1;
    ui->BufferWindow->m_channelHot[3] = 1;
    ui->BufferWindow->UpdateOCIOGLState();
    ui->BufferWindow->updateGL();
}


void MainWindow::on_EvButton_clicked()
{
    ui->EvSlider->setValue(0);
}

void MainWindow::on_YButton_clicked()
{
    ui->YSlider->setValue(100);
}

void MainWindow::on_historyList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(current){
        std::cout <<  current->text().toStdString().c_str() << ", " << ui->historyList->currentRow() << ImagesBuffers[ui->historyList->currentRow()].getName().c_str()<< std::endl;
        ui->BufferWindow->InitImageTexture(ImagesBuffers[ui->historyList->currentRow()]);
    }
}
