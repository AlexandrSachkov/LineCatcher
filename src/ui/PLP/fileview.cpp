#include "fileview.h"
#include <QtWidgets/QVBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

FileView::FileView(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    std::wstring path;
    fileReader->getFilePath(path);
    _fileInfo = new QLabel(QString::fromStdWString(path), this);
    mainLayout->addWidget(_fileInfo);

    _fileNavControls = new FileNavControls(this);
    mainLayout->addWidget(_fileNavControls);

    _dataView = new PagedFileViewWidget(std::move(fileReader), this);
    _dataView->setReadOnly(true);
    mainLayout->addWidget(_dataView);
}

FileView::~FileView() {
    bool zz = true;
}
