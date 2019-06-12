#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>
#include <functional>

class SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsDialog(const std::function<void(int)>& changeViewerFont, QWidget* parent = nullptr);


};

#endif // SETTINGSDIALOG_H
