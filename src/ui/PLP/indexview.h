#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <QWidget>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
#include "ullspinbox.h"
#include "ResultSetReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexView : public QWidget
{
    Q_OBJECT
public:
    explicit IndexView(
        CoreObjPtr<PLP::ResultSetReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        QWidget *parent = nullptr
    );

signals:

public slots:
};

#endif // INDEXVIEW_H
