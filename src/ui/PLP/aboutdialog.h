#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QWidget>
#include <QLabel>

class AboutDialog : public QWidget
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

private slots:
    void onOk();

};

#endif // ABOUTDIALOG_H
