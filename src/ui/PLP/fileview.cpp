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
    mainLayout->setContentsMargins(0, 0, 0, 0);

    std::wstring path;
    fileReader->getFilePath(path);
    _filePath = QString::fromStdWString(path);

    QHBoxLayout* lineSelectionLayout = new QHBoxLayout();
    mainLayout->addLayout(lineSelectionLayout);

    QFont lineNavFont("Courier New", 16);
    lineNavFont.setStyleHint(QFont::Monospace);

    //lineSelectionLayout->addWidget(new QLabel("Line", this), 1, Qt::AlignRight);
    _currLineNumBox = new ULLSpinBox(this);
    _currLineNumBox->setFont(lineNavFont);
    _currLineNumBox->setRange(0, fileReader->getNumberOfLines() - 1);
    _currLineNumBox->setValue(0);
    lineSelectionLayout->addWidget(_currLineNumBox, 1, Qt::AlignRight);

    QLabel* numLinesLabel = new QLabel("/" + QString::number(fileReader->getNumberOfLines() - 1), this);
    numLinesLabel->setFont(lineNavFont);
    numLinesLabel->setContentsMargins(0, 0, 5, 0);
    lineSelectionLayout->addWidget(numLinesLabel, 0);

    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setContentsMargins(0,0,0,0);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(5);
    splitter->setChildrenCollapsible(false);
    mainLayout->addWidget(splitter);

    _dataView = new PagedFileViewWidget(std::move(fileReader), _currLineNumBox, splitter);
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
