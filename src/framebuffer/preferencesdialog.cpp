#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include <QColorDialog>

#include <iostream>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_actionPickColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::green, this);
    std::cout << color.redF() << ", " << color.greenF() << ", " << color.blueF() << std::endl;

}
