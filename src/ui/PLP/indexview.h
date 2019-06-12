#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <QWidget>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
#include "indexviewwidget.h"
#include "ullspinbox.h"
#include "IndexReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexView : public QWidget
{
    Q_OBJECT
public:
    explicit IndexView(
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        QWidget *parent = nullptr
    );

    const QString& getFilePath();
    void setFontSize(int pointSize);

private:
    QString _filePath;
    IndexViewWidget* _indexView;
};

#endif // INDEXVIEW_H
