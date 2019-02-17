#include "fileview.h"
#include <QtWidgets/QVBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

FileView::FileView(const QString& filePath, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    _fileInfo = new QLabel(filePath, this);
    mainLayout->addWidget(_fileInfo);

    _fileNavControls = new FileNavControls(this);
    mainLayout->addWidget(_fileNavControls);

    _dataView = new PagedFileViewWidget(nullptr, this);
    _dataView->setReadOnly(true);
    mainLayout->addWidget(_dataView);

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        _dataView->setPlainText(file.readAll());
    }
}

FileView::~FileView() {

}
