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

    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(5);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    _dataView = new PagedFileViewWidget(std::move(fileReader), splitter);
    _dataView->setReadOnly(true);
    splitter->addWidget(_dataView);

    _resultSetViewer = new QTabWidget(splitter);
    _resultSetViewer->setTabsClosable(true);
    splitter->addWidget(_resultSetViewer);
    connect(_resultSetViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    _resultSetViewer->addTab(new QWidget(), "Tab test");


}

FileView::~FileView() {
    bool zz = true;
}

void FileView::closeTab(int index) {
    if (index == -1) {
        return;
    }

    QWidget* tab = _resultSetViewer->widget(index);
    if(tab){
        delete tab;
    }
    _resultSetViewer->removeTab(index);
}
