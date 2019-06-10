#include "fileview.h"
#include "indexview.h"
#include "indexviewwidget.h"
#include "ullspinbox.h"
#include <QtWidgets/QVBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>

FileView::FileView(CoreObjPtr<PLP::FileReaderI> fileReader, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    std::wstring path(fileReader->getFilePath());
    _filePath = QString::fromStdWString(path);

    QHBoxLayout* lineSelectionLayout = new QHBoxLayout();
    mainLayout->addLayout(lineSelectionLayout);

    //QFont lineNavFont("Courier New", 14);
    //lineNavFont.setStyleHint(QFont::Monospace);

    _currLineNumBox = new ULLSpinBox(this);
    //_currLineNumBox->setFont(lineNavFont);
    unsigned long long maxRange = fileReader->getNumberOfLines() > 0 ? fileReader->getNumberOfLines() - 1 : 0;
    _currLineNumBox->setRange(0, maxRange);
    lineSelectionLayout->addWidget(_currLineNumBox, 1, Qt::AlignRight);

    QLabel* numLinesLabel = new QLabel("/" + QString::number(fileReader->getNumberOfLines()), this);
    //numLinesLabel->setFont(lineNavFont);
    numLinesLabel->setContentsMargins(0, 0, 5, 0);
    lineSelectionLayout->addWidget(numLinesLabel, 0);

    _splitter = new QSplitter(this);
    mainLayout->addWidget(_splitter);
    _splitter->setOrientation(Qt::Orientation::Vertical);
    _splitter->setHandleWidth(5);
    _splitter->setChildrenCollapsible(false);

    _dataView = new PagedFileViewWidget(std::move(fileReader), _currLineNumBox, _splitter);
    _dataView->setReadOnly(true);
    _splitter->addWidget(_dataView);

    _indexViewer = new QTabWidget(_splitter);
    _indexViewer->setTabsClosable(true);
    _splitter->addWidget(_indexViewer);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    _splitter->setSizes(QList<int>({screenGeometry.height(), 0}));

    connect(_indexViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

FileView::~FileView() {
    delete _dataView;
    delete _indexViewer;
}

void FileView::closeTab(int index) {
    if (index == -1) {
        return;
    }

    QWidget* tab = _indexViewer->widget(index);
    if(tab){
        delete tab;
    }

    if(_indexViewer->count() == 0){
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        _splitter->setSizes(QList<int>({screenGeometry.height(), 0}));
    }
}

const QString& FileView::getFilePath() {
    return _filePath;
}

void FileView::openIndex(CoreObjPtr<PLP::IndexReaderI> indexReader){
    std::wstring path(indexReader->getFilePath());
    QString qPath = QString::fromStdWString(path);

    int numTabs = _indexViewer->count();
    for(int i = 0; i < numTabs; i++){
        IndexView* indexView = static_cast<IndexView*>(_indexViewer->widget(i));
        const QString& existingPath = indexView->getFilePath();
        if(qPath.compare(existingPath) == 0){
            _indexViewer->setCurrentIndex(i);
            return;
        }
    }

    QString fileName = qPath.split('/').last();
    IndexView* indexView = new IndexView(std::move(indexReader), _dataView, this);
    _indexViewer->addTab(indexView, fileName);
    _indexViewer->setTabToolTip(_indexViewer->count() - 1, qPath);
    _indexViewer->setCurrentIndex(_indexViewer->count() - 1);

    if(_indexViewer->count() == 1){
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        _splitter->setSizes(QList<int>({screenGeometry.height() / 4 * 3, screenGeometry.height() / 4}));
    }
}
