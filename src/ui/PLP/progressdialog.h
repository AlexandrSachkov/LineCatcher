#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>

class ProgressDialog : public QDialog
{
    Q_OBJECT
public:
    ProgressDialog(QWidget* parent = nullptr);
};

#endif // PROGRESSDIALOG_H
