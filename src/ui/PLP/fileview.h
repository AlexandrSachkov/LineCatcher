#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "filenavcontrols.h"
#include "pagedfileviewwidget.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>

#include "FileReaderI.h"


class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent = nullptr);
    ~FileView();
signals:

public slots:

private:
    PagedFileViewWidget* _dataView;
    QLabel* _fileInfo;
    FileNavControls* _fileNavControls;
};

#endif // FILEVIEW_H
