#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QWidget>

class SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

signals:

private slots:
    void changeUIFontSize(int size);

};

#endif // SETTINGSDIALOG_H
