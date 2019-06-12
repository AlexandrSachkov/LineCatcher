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
