#include "indexview.h"
#include "indexviewwidget.h"
#include <QVBoxLayout>
#include <QLabel>

IndexView::IndexView(
    std::unique_ptr<PLP::ResultSetReaderI> indexReader,
    PagedFileViewWidget* fileViewer,
    QWidget* parent
    ) : QWidget(parent) {

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QFont lineNavFont("Courier New", 14);
    lineNavFont.setStyleHint(QFont::Monospace);

    QHBoxLayout* lineSelectionLayout = new QHBoxLayout();
    mainLayout->addLayout(lineSelectionLayout);

    ULLSpinBox* lineNavBox = new ULLSpinBox(this);
    lineNavBox->setFont(lineNavFont);
    lineNavBox->setRange(0, indexReader->getNumResults() - 1);
    lineSelectionLayout->addWidget(lineNavBox, 1, Qt::AlignRight);

    QLabel* numLinesLabel = new QLabel("/" + QString::number(indexReader->getNumResults() - 1), this);
    numLinesLabel->setFont(lineNavFont);
    numLinesLabel->setContentsMargins(0, 0, 5, 0);
    lineSelectionLayout->addWidget(numLinesLabel, 0);

    IndexViewWidget* indexView = new IndexViewWidget(std::move(indexReader), fileViewer, lineNavBox, this);
    mainLayout->addWidget(indexView);
}
