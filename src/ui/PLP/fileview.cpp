#include "fileview.h"
#include "indexviewwidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>

FileView::FileView(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);

    std::wstring path;
    fileReader->getFilePath(path);
    _filePath = QString::fromStdWString(path);

    _fileNavControls = new FileNavControls(this);
    mainLayout->addWidget(_fileNavControls);

    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(5);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    _dataView = new PagedFileViewWidget(std::move(fileReader), splitter);
    _dataView->setReadOnly(true);
    splitter->addWidget(_dataView);

    _indexViewer = new QTabWidget(splitter);
    _indexViewer->setTabsClosable(true);
    splitter->addWidget(_indexViewer);
    connect(_indexViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

FileView::~FileView() {
    bool zz = true;
}

void FileView::closeTab(int index) {
    if (index == -1) {
        return;
    }

    QWidget* tab = _indexViewer->widget(index);
    if(tab){
        delete tab;
    }
}

const QString& FileView::getFilePath() {
    return _filePath;
}

void FileView::openIndex(std::unique_ptr<PLP::ResultSetReaderI> indexReader){
    std::wstring path;
    indexReader->getFilePath(path);
    QString qPath = QString::fromStdWString(path);
    QString fileName = qPath.split('/').last();

    IndexViewWidget* indexView = new IndexViewWidget(std::move(indexReader), _dataView, this);
    _indexViewer->addTab(indexView, fileName);
    _indexViewer->setTabToolTip(_indexViewer->count() - 1, qPath);
    _indexViewer->setCurrentIndex(_indexViewer->count() - 1);
}
