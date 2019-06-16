#include "settingsdialog.h"
#include "mainwindow.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QSlider>
#include <QSettings>

SettingsDialog::SettingsDialog(
        const std::function<void(int)>& changeViewerFont,
        QWidget* parent) : QWidget(parent)
{
    QFormLayout* mainLayout = new QFormLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModal);
    setMinimumWidth(400);
    setMinimumHeight(300);

    _viewerFontSize = new QSlider(Qt::Orientation::Horizontal, this);
    mainLayout->addRow("Viewer font size:", _viewerFontSize);
    _viewerFontSize->setTickPosition(QSlider::TickPosition::TicksBelow);
    _viewerFontSize->setTickInterval(1);
    _viewerFontSize->setMinimum(10);
    _viewerFontSize->setMaximum(24);
    connect(_viewerFontSize, &QSlider::valueChanged, [changeViewerFont](int value){
        changeViewerFont(value);
    });

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);

    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("Settings");

    int viewerFontSizeVal = settings.value("viewerFontSize", 14).toInt();
    QMetaObject::invokeMethod(_viewerFontSize, [&, viewerFontSizeVal](){
        _viewerFontSize->setValue(viewerFontSizeVal);
        changeViewerFont(viewerFontSizeVal);
    });

    settings.endGroup();
}

void SettingsDialog::hideEvent(QHideEvent* event) {
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("Settings");
    settings.setValue("viewerFontSize", _viewerFontSize->value());
    settings.endGroup();
}
