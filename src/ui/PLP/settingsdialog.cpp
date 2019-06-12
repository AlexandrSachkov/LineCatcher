#include "settingsdialog.h"
#include "mainwindow.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QSlider>

SettingsDialog::SettingsDialog(
        const std::function<void(int)>& changeViewerFont,
        QWidget* parent) : QWidget(parent)
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

    QSlider* contentFontSize = new QSlider(Qt::Orientation::Horizontal, this);
    mainLayout->addRow("Viewer font size:", contentFontSize);
    contentFontSize->setTickPosition(QSlider::TickPosition::TicksBelow);
    contentFontSize->setTickInterval(1);
    contentFontSize->setMinimum(10);
    contentFontSize->setMaximum(24);
    connect(contentFontSize, &QSlider::valueChanged, [changeViewerFont](int value){
        changeViewerFont(value);
    });

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);

    QMetaObject::invokeMethod(contentFontSize, [&](){
        contentFontSize->setValue(14);
    });
}
