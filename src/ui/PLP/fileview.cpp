/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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

    QHBoxLayout* optionsLayout = new QHBoxLayout();
    mainLayout->addLayout(optionsLayout);

    _highlights = new HighlightsDialog([&](){
        _dataView->onHighlightListUpdated();
    }, this);
    _highlights->hide();

    QPushButton* showHighlights = new QPushButton("Highlights", this);
    showHighlights->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    showHighlights->setToolTip("(Ctrl + E)");
    optionsLayout->addWidget(showHighlights);  
    connect(showHighlights, &QPushButton::clicked, [&, this](){
        _highlights->show();
    });

    QFont lineNavFont("Courier New", 12);
    lineNavFont.setStyleHint(QFont::Monospace);

    _currLineNumBox = new ULLSpinBox(this);
    _currLineNumBox->setFont(lineNavFont);
    unsigned long long maxRange = fileReader->getNumberOfLines() > 0 ? fileReader->getNumberOfLines() - 1 : 0;
    _currLineNumBox->setRange(0, maxRange);
    optionsLayout->addWidget(_currLineNumBox, 1, Qt::AlignRight);

    QLabel* numLinesLabel = new QLabel("/" + QString::number(fileReader->getNumberOfLines()), this);
    numLinesLabel->setFont(lineNavFont);
    numLinesLabel->setContentsMargins(0, 0, 5, 0);
    optionsLayout->addWidget(numLinesLabel, 0);

    _splitter = new QSplitter(this);
    mainLayout->addWidget(_splitter);
    _splitter->setOrientation(Qt::Orientation::Vertical);
    _splitter->setHandleWidth(5);
    _splitter->setChildrenCollapsible(false);

    _dataView = new PagedFileViewWidget(std::move(fileReader), _currLineNumBox,
        [&](){
            return _highlights->getHighlights();
        }, [&](const QString& text){
            _highlights->addHighlight(text, false);
        }, [&](){
            _highlights->clear();
        },
        _splitter
    );

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

void FileView::closeCurrentIndex() {
    closeTab(_indexViewer->currentIndex());
}

void FileView::closeIndex(const QString& path) {
    int numTabs = _indexViewer->count();
    for(int i = 0; i < numTabs; i++){
        IndexView* indexView = static_cast<IndexView*>(_indexViewer->widget(i));
        const QString& existingPath = indexView->getFilePath();
        if(existingPath == path)
        {
            closeTab(i);
            return;
        }
    }
}

const QString& FileView::getFilePath() {
    return _filePath;
}

void FileView::openIndex(CoreObjPtr<PLP::IndexReaderI> indexReader, const QString& highlightPattern, bool regex) {
    openIndex(std::move(indexReader));
    _highlights->addHighlight(highlightPattern, regex);
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

void FileView::setFontSize(int pointSize) {
    _dataView->setFontSize(pointSize);

    for(int i = 0; i < _indexViewer->count(); i++){
        IndexView* indexView = static_cast<IndexView*>(_indexViewer->widget(i));
        indexView->setFontSize(pointSize);
    }
}
