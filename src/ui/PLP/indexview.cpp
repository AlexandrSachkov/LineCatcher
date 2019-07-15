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

#include "indexview.h"
#include "indexviewwidget.h"
#include <QVBoxLayout>
#include <QLabel>

IndexView::IndexView(
    CoreObjPtr<PLP::IndexReaderI> indexReader,
    PagedFileViewWidget* fileViewer,
    QWidget* parent
    ) : QWidget(parent) {

    std::wstring path(indexReader->getFilePath());
    _filePath = QString::fromStdWString(path);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QFont lineNavFont("Courier New", 12);
    lineNavFont.setStyleHint(QFont::Monospace);

    QHBoxLayout* lineSelectionLayout = new QHBoxLayout();
    mainLayout->addLayout(lineSelectionLayout);

    ULLSpinBox* lineNavBox = new ULLSpinBox(this);
    lineNavBox->setFont(lineNavFont);
    unsigned long long maxRange = indexReader->getNumResults() > 0 ? indexReader->getNumResults() - 1 : 0;
    lineNavBox->setRange(0, maxRange);
    lineSelectionLayout->addWidget(lineNavBox, 1, Qt::AlignRight);

    QLabel* numLinesLabel = new QLabel("/" + QString::number(indexReader->getNumResults()), this);
    numLinesLabel->setFont(lineNavFont);
    numLinesLabel->setContentsMargins(0, 0, 5, 0);
    lineSelectionLayout->addWidget(numLinesLabel, 0);

    _indexView = new IndexViewWidget(std::move(indexReader), fileViewer, lineNavBox, this);
    mainLayout->addWidget(_indexView);
}

const QString& IndexView::getFilePath(){
    return _filePath;
}

void IndexView::setFontSize(int pointSize) {
    _indexView->setFontSize(pointSize);
}
