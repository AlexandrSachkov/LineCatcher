#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <QWidget>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
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

signals:

public slots:

private:
    QString _filePath;
};

#endif // INDEXVIEW_H
