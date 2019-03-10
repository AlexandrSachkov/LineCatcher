#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "pagedfileviewwidget.h"
#include "ullspinbox.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>

#include "FileReaderI.h"
#include "ResultSetReaderI.h"

#include <memory>

class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent = nullptr);
    ~FileView();

    const QString& getFilePath();
    void openIndex(std::unique_ptr<PLP::ResultSetReaderI> indexReader);
signals:

private slots:
    void closeTab(int index);

private:
    PagedFileViewWidget* _dataView;
    QTabWidget* _indexViewer;
    ULLSpinBox* _currLineNumBox;

    QString _filePath;
};

#endif // FILEVIEW_H
