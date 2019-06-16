#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <functional>

#include "CoreI.h"

class SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsDialog(PLP::CoreI* plpCore, const std::function<void(int)>& changeViewerFont, QWidget* parent = nullptr);

protected:
    void hideEvent(QHideEvent* event);

private:
    PLP::CoreI* _plpCore = nullptr;
    QSlider* _viewerFontSize;
    QCheckBox* _deleteMetaFiles;
};

#endif // SETTINGSDIALOG_H
