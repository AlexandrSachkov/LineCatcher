#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <QWidget>

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
        std::unique_ptr<PLP::ResultSetReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        QWidget *parent = nullptr
    );

signals:

public slots:
};

#endif // INDEXVIEW_H
