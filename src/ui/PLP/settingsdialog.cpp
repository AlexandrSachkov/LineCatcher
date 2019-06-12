#include "settingsdialog.h"
#include "mainwindow.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QSlider>

SettingsDialog::SettingsDialog(QWidget *parent) : QWidget(parent)
{
    QFormLayout* mainLayout = new QFormLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    setWindowFlags(Qt::Dialog);
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModal);
    setMinimumWidth(400);
    setMinimumHeight(300);

    QSlider* uiFontSize = new QSlider(Qt::Orientation::Horizontal, this);
    mainLayout->addRow("UI font size:", uiFontSize);
    uiFontSize->setTickPosition(QSlider::TickPosition::TicksBothSides);
    uiFontSize->setTickInterval(1);
    uiFontSize->setMinimum(10);
    uiFontSize->setMaximum(20);
    uiFontSize->setValue(12);
    connect(uiFontSize, &QSlider::valueChanged,this, &SettingsDialog::changeUIFontSize);

    QSlider* contentFontSize = new QSlider(Qt::Orientation::Horizontal, this);
    mainLayout->addRow("Content font size:", contentFontSize);
    contentFontSize->setTickPosition(QSlider::TickPosition::TicksBothSides);
    contentFontSize->setTickInterval(1);
    contentFontSize->setMinimum(10);
    contentFontSize->setMaximum(20);
    contentFontSize->setValue(12);

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);
}

void SettingsDialog::changeUIFontSize(int size){
    MainWindow* main = static_cast<MainWindow*>(parent());
    QFont f = main->font();
    f.setPointSize(size);
    main->setFont(f);
}
