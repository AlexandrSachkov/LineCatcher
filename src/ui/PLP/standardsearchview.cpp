#include "standardsearchview.h"
#include "ullspinbox.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>

StandardSearchView::StandardSearchView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);
    setWindowTitle("Search");

    QFormLayout* contentLayout = new QFormLayout();
    mainLayout->addLayout(contentLayout);

    QLineEdit* filePath = new QLineEdit(this);
    filePath->setReadOnly(true);
    QPushButton* openFile = new QPushButton("Open", this);
    openFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout* openFileLayout = new QHBoxLayout();
    openFileLayout->addWidget(filePath);
    openFileLayout->addWidget(openFile);
    contentLayout->addRow("File: ", openFileLayout);

    QLineEdit* indexPath = new QLineEdit(this);
    indexPath->setReadOnly(true);
    QPushButton* openIndex = new QPushButton("Open", this);
    openIndex->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout* openIndexLayout = new QHBoxLayout();
    openIndexLayout->addWidget(indexPath);
    openIndexLayout->addWidget(openIndex);
    contentLayout->addRow("Index (Optional): ", openIndexLayout);

    // Line range
    QLabel* fromLineLabel = new QLabel("From:", this);
    ULLSpinBox* fromLineBox = new ULLSpinBox(this);
    fromLineBox->setRange(0, ULLONG_MAX);
    fromLineLabel->setBuddy(fromLineBox);

    QLabel* toLineLabel = new QLabel("To:", this);
    ULLSpinBox* toLineBox = new ULLSpinBox(this);
    toLineBox->setRange(0, ULLONG_MAX);
    toLineLabel->setBuddy(toLineBox);

    QHBoxLayout* lineRangeLayout = new QHBoxLayout();
    lineRangeLayout->addWidget(fromLineLabel);
    lineRangeLayout->addWidget(fromLineBox);
    lineRangeLayout->addWidget(toLineLabel);
    lineRangeLayout->addWidget(toLineBox);
    contentLayout->addRow("Line Range: ", lineRangeLayout);

    // Num results
    ULLSpinBox* numResultsBox = new ULLSpinBox(this);
    numResultsBox->setRange(0, ULLONG_MAX);
    contentLayout->addRow("Max # results: ", numResultsBox);

    QLineEdit* searchField = new QLineEdit(this);
    contentLayout->addRow("Search pattern: ", searchField);

    QGroupBox* groupBox = new QGroupBox("Search pattern options", this);
    QVBoxLayout* searchOptionsLayout = new QVBoxLayout();
    groupBox->setLayout(searchOptionsLayout);
    contentLayout->addWidget(groupBox);

    QRadioButton* plainText = new QRadioButton("Plain text", this);
    plainText->setChecked(true);
    QRadioButton* regex = new QRadioButton("Regex", this);
    QHBoxLayout* plainTextRegexLayout = new QHBoxLayout();
    plainTextRegexLayout->addWidget(plainText);
    plainTextRegexLayout->addWidget(regex);
    searchOptionsLayout->addLayout(plainTextRegexLayout);

    QCheckBox* ignoreCase = new QCheckBox("Ignore case", this);
    searchOptionsLayout->addWidget(ignoreCase);

    QPushButton* runSearch = new QPushButton("Search", this);
    contentLayout->addWidget(runSearch);

    QFont f("Courier New", 14);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);
}
