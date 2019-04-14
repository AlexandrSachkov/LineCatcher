#include "standardsearchview.h"
#include "ullspinbox.h"
#include "common.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpacerItem>

StandardSearchView::StandardSearchView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    setWindowFlags(Qt::Window);
    setWindowTitle("Search");

    createSourceContent(mainLayout);
    createDestinationContent(mainLayout);
    createSearchLimiterContent(mainLayout);
    createSearchOptionContent(mainLayout);

    QPushButton* runSearch = new QPushButton("Search", this);
    mainLayout->addWidget(runSearch);

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);
}

void StandardSearchView::createSourceContent(QLayout* mainLayout){
    QGroupBox* sourceGroup = new QGroupBox("Source", this);
    sourceGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(sourceGroup);

    QFormLayout* sourceLayout = new QFormLayout();
    sourceGroup->setLayout(sourceLayout);

    QHBoxLayout* openFileLayout = new QHBoxLayout();
    QLineEdit* filePath = new QLineEdit(this);
    filePath->setReadOnly(true);
    QPushButton* openFile = new QPushButton("Open", this);
    openFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openFileLayout->addWidget(filePath);
    openFileLayout->addWidget(openFile);
    sourceLayout->addRow("File: ", openFileLayout);

    QHBoxLayout* openIndexLayout = new QHBoxLayout();
    QLineEdit* indexPath = new QLineEdit(this);
    indexPath->setReadOnly(true);
    QPushButton* openIndex = new QPushButton("Open", this);
    openIndex->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openIndexLayout->addWidget(indexPath);
    openIndexLayout->addWidget(openIndex);

    sourceLayout->addRow("Index (Optional): ", openIndexLayout);
}

void StandardSearchView::createDestinationContent(QLayout* mainLayout){
    QGroupBox* destGroup = new QGroupBox("Save results to", this);
    destGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(destGroup);

    //Open file
    QFormLayout* destLayout = new QFormLayout();
    destGroup->setLayout(destLayout);

    QLineEdit* name = new QLineEdit(this);
    destLayout->addRow("File name: ", name);
}

void StandardSearchView::createSearchLimiterContent(QLayout* mainLayout){
    QGroupBox* limiterGroup = new QGroupBox("Limiters", this);
    limiterGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(limiterGroup);

    QFormLayout* limiterLayout = new QFormLayout();
    limiterGroup->setLayout(limiterLayout);

    ULLSpinBox* fromLineBox = new ULLSpinBox(this);
    fromLineBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("Start line#:", fromLineBox);

    ULLSpinBox* toLineBox = new ULLSpinBox(this);
    toLineBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("End line#:", toLineBox);

    ULLSpinBox* numResultsBox = new ULLSpinBox(this);
    numResultsBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("Max result#: ", numResultsBox);
}

void StandardSearchView::createSearchOptionContent(QLayout* mainLayout) {
    QGroupBox* searchGroup = new QGroupBox("Search", this);
    searchGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(searchGroup);

    QFormLayout* searchLayout = new QFormLayout();
    searchGroup->setLayout(searchLayout);

    QLineEdit* searchField = new QLineEdit(this);
    searchLayout->addRow("Pattern: ", searchField);

    QRadioButton* plainText = new QRadioButton("Plain text", this);
    plainText->setChecked(true);
    QRadioButton* regex = new QRadioButton("Regex", this);
    QHBoxLayout* plainTextRegexLayout = new QHBoxLayout();
    plainTextRegexLayout->addWidget(plainText);
    plainTextRegexLayout->addWidget(regex);
    searchLayout->addRow(plainTextRegexLayout);

    QCheckBox* ignoreCase = new QCheckBox("Ignore case", this);
    searchLayout->addRow(ignoreCase);
}
