#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <QSlider>
#include <functional>

class SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsDialog(const std::function<void(int)>& changeViewerFont, QWidget* parent = nullptr);

protected:
    void hideEvent(QHideEvent* event);

private:
    QSlider* _viewerFontSize;
};

#endif // SETTINGSDIALOG_H
