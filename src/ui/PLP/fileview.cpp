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

    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(10);
    splitter->setChildrenCollapsible(false);

    _dataView = new QTextEdit(splitter);
    _dataView->setReadOnly(true);
    splitter->addWidget(_dataView);

    _searchResults = new QTabWidget(splitter);
    _searchResults->setTabsClosable(true);
    splitter->addWidget(_searchResults);

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        _dataView->setText(file.readAll());
    }

    _searchResults->addTab(new QWidget(), "Tab test");
}

FileView::~FileView() {

}
