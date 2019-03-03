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

private slots:
    void closeTab(int index);

private:
    PagedFileViewWidget* _dataView;
    QLabel* _fileInfo;
    FileNavControls* _fileNavControls;
    QTabWidget* _resultSetViewer;
};

#endif // FILEVIEW_H
