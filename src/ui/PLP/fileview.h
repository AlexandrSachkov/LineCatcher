#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "filenavcontrols.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>


class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(const QString& filePath, QWidget *parent = nullptr);
    ~FileView();
signals:

public slots:

private:
    QPlainTextEdit* _dataView;
    QLabel* _fileInfo;
    FileNavControls* _fileNavControls;
};

#endif // FILEVIEW_H
