#include "searchview.h"
#include "common.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QSpacerItem>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSpinBox>
#include <QGridLayout>

SearchView::SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent) : QWidget(parent)
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

    if(multiline){
        createMultilineSearchOptionContent(mainLayout);
    }else{
        createSearchOptionContent(mainLayout);
    }

    QPushButton* runSearch = new QPushButton("Search", this);
    connect(runSearch, SIGNAL(clicked(void)), this, SLOT(startSearch(void)));
    mainLayout->addWidget(runSearch);

    connect(this, SIGNAL(progressUpdate(int, unsigned long long)), this, SLOT(onProgressUpdate(int, unsigned long long)));
    connect(this, SIGNAL(searchError(void)), this, SLOT(onSearchError(void)));

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);
}

void SearchView::createSourceContent(QLayout* mainLayout){
    QGroupBox* sourceGroup = new QGroupBox("Source", this);
    sourceGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(sourceGroup);

    QFormLayout* sourceLayout = new QFormLayout();
    sourceGroup->setLayout(sourceLayout);

    QHBoxLayout* openFileLayout = new QHBoxLayout();
    _filePath = new QLineEdit(this);
    _filePath->setReadOnly(true);
    QPushButton* openFile = new QPushButton("Open", this);
    connect(openFile, SIGNAL(clicked(void)), this, SLOT(openFile(void)));
    openFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openFileLayout->addWidget(_filePath);
    openFileLayout->addWidget(openFile);
    sourceLayout->addRow("File: ", openFileLayout);

    QHBoxLayout* openIndexLayout = new QHBoxLayout();
    _indexPath = new QLineEdit(this);
    _indexPath->setReadOnly(true);
    QPushButton* openIndex = new QPushButton("Open", this);
    connect(openIndex, SIGNAL(clicked(void)), this, SLOT(openIndex(void)));
    openIndex->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openIndexLayout->addWidget(_indexPath);
    openIndexLayout->addWidget(openIndex);

    sourceLayout->addRow("Index (Optional): ", openIndexLayout);
}

void SearchView::createDestinationContent(QLayout* mainLayout){
    QGroupBox* destGroup = new QGroupBox("Save results to", this);
    destGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(destGroup);

    //Open file
    QFormLayout* destLayout = new QFormLayout();
    destGroup->setLayout(destLayout);

    _destName = new QLineEdit(this);
    destLayout->addRow("File name: ", _destName);
}

void SearchView::createSearchLimiterContent(QLayout* mainLayout){
    QGroupBox* limiterGroup = new QGroupBox("Limiters", this);
    limiterGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(limiterGroup);

    QFormLayout* limiterLayout = new QFormLayout();
    limiterGroup->setLayout(limiterLayout);

    _fromLineBox = new ULLSpinBox(this);
    _fromLineBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("Start line#:", _fromLineBox);

    _toLineBox = new ULLSpinBox(this);
    _toLineBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("End line#:", _toLineBox);

    _numResultsBox = new ULLSpinBox(this);
    _numResultsBox->setRange(0, ULLONG_MAX);
    limiterLayout->addRow("Max result#: ", _numResultsBox);
}

void SearchView::createSearchOptionContent(QLayout* mainLayout) {
    QGroupBox* searchGroup = new QGroupBox("Search", this);
    searchGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(searchGroup);

    QFormLayout* searchLayout = new QFormLayout();
    searchGroup->setLayout(searchLayout);

    _searchField = new QLineEdit(this);
    searchLayout->addRow("Pattern: ", _searchField);

    _plainText = new QRadioButton("Plain text", this);
    _plainText->setChecked(true);
    _regex = new QRadioButton("Regex", this);
    QHBoxLayout* plainTextRegexLayout = new QHBoxLayout();
    plainTextRegexLayout->addWidget(_plainText);
    plainTextRegexLayout->addWidget(_regex);
    searchLayout->addRow(plainTextRegexLayout);

    _ignoreCase = new QCheckBox("Ignore case", this);
    searchLayout->addRow(_ignoreCase);
}

void SearchView::createMultilineSearchOptionContent(QLayout* mainLayout){
    QGroupBox* searchGroup = new QGroupBox("Search", this);
    searchGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(searchGroup);

    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(2);
    searchGroup->setLayout(formLayout);

    const int NUM_ROWS = 5;

    //Line offset
    {
        QLabel* lineOffsetLabel = new QLabel("Line #", this);
        lineOffsetLabel->setAlignment(Qt::AlignHCenter);
        formLayout->addWidget(lineOffsetLabel, 0, 0);

        for(int i = 0; i < NUM_ROWS; i++){
            QSpinBox* lineOffsetBox = new QSpinBox(this);
            lineOffsetBox->setRange(-10, 10);
            lineOffsetBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
            _lineOffsetBoxes.push_back(lineOffsetBox);
            formLayout->addWidget(lineOffsetBox, i + 1, 0);
        }
    }

    //Word offset
    {
        QLabel* wordOffsetLabel = new QLabel("Word # (Optional)", this);
        wordOffsetLabel->setAlignment(Qt::AlignHCenter);
        formLayout->addWidget(wordOffsetLabel, 0, 1);

        for(int i = 0; i < NUM_ROWS; i++){
            QSpinBox* wordOffsetBox = new QSpinBox(this);
            wordOffsetBox->setRange(-1000, 1000);
            wordOffsetBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
            _wordOffsetBoxes.push_back(wordOffsetBox);
            formLayout->addWidget(wordOffsetBox, i + 1, 1);
        }
    }

    //Search pattern
    {
        QLabel* searchPatternLabel = new QLabel("Search pattern", this);
        searchPatternLabel->setAlignment(Qt::AlignHCenter);
        formLayout->addWidget(searchPatternLabel, 0, 2);

        for(int i = 0; i < NUM_ROWS; i++){
            QLineEdit* searchPatternBox = new QLineEdit(this);
            _searchPatternBoxes.push_back(searchPatternBox);
            formLayout->addWidget(searchPatternBox, i + 1, 2);
        }
    }

    //Plain text / Regex
    {
        for(int i = 0; i < NUM_ROWS; i++){
            QGroupBox* plainTextRegexGroup = new QGroupBox("", this);
            plainTextRegexGroup->setStyleSheet(PLAINTEXT_REGEX_GROUP_STYLESHEET);
            QHBoxLayout* plainTextRegexGroupLayout = new QHBoxLayout();
            plainTextRegexGroup->setLayout(plainTextRegexGroupLayout);

            QRadioButton* plainTextButton = new QRadioButton("Plain text", this);
            plainTextButton->setChecked(true);
            _plainTextButtons.push_back(plainTextButton);
            plainTextRegexGroupLayout->addWidget(plainTextButton);

            QRadioButton* regexButton = new QRadioButton("Regex", this);
            _regexButtons.push_back(regexButton);
            plainTextRegexGroupLayout->addWidget(regexButton);

            formLayout->addWidget(plainTextRegexGroup, i + 1, 3);
        }
    }

    //Ignore case
    {
        for(int i = 0; i < NUM_ROWS; i++){
            QCheckBox* ignoreCaseCheckBox = new QCheckBox("Ignore case", this);
            _ignoreCaseCheckBoxes.push_back(ignoreCaseCheckBox);
            formLayout->addWidget(ignoreCaseCheckBox, i + 1, 4);
        }
    }
}

void SearchView::startSearch() {
    QString dataPath = _filePath->text();
    QString indexPath = _indexPath->text();
    QString destPath = _destName->text();
    unsigned long long startLine = _fromLineBox->value();
    unsigned long long endLine = _toLineBox->value();
    unsigned long long maxNumResults = _numResultsBox->value();
    QString searchPattern = _searchField->text();
    bool plainText = _plainText->isChecked();
    bool ignoreCase = _ignoreCase->isChecked();

    //validate user input
    if(dataPath.simplified().isEmpty()){
        QMessageBox::information(this,"PLP","Source file path cannot be empty",QMessageBox::Ok);
        return;
    }

    if(destPath.simplified().isEmpty()){
        QMessageBox::information(this,"PLP","Save to file name cannot be empty",QMessageBox::Ok);
        return;
    }

    if(searchPattern.isEmpty()){
        QMessageBox::information(this,"PLP","Search pattern cannot be empty",QMessageBox::Ok);
        return;
    }

    // Create required objects
    PLP::FileReaderI* fileReader = _plpCore->createFileReader(dataPath.toStdString(), 0, true);
    if(!fileReader){
        QMessageBox::information(this,"PLP","File reader failed to initialize",QMessageBox::Ok);
        return;
    }

    PLP::ResultSetReaderI* indexReader = nullptr;
    if(!indexPath.simplified().isEmpty()){
        indexReader = _plpCore->createResultSetReader(indexPath.toStdString(), 0);
        if(!indexReader){
            QMessageBox::information(this,"PLP","Index reader failed to initialize",QMessageBox::Ok);
            _plpCore->release(fileReader);
            return;
        }
    }
    PLP::ResultSetWriterI* indexWriter = _plpCore->createResultSetWriter(destPath.toStdString(), 0, fileReader, true);
    if(!indexWriter){
        QMessageBox::information(this,"PLP","Index writer failed to initialize",QMessageBox::Ok);
        _plpCore->release(fileReader);
        _plpCore->release(indexReader);
        return;
    }

    PLP::TextComparator* comparator = nullptr;
    if(plainText){
        comparator = new PLP::MatchString(searchPattern.toStdString(), false, ignoreCase);
    }else{
        comparator = new PLP::MatchRegex(searchPattern.toStdString(), ignoreCase);
    }

    if(!comparator || !comparator->initialize()){
        QMessageBox::information(this,"PLP","Comparator failed to initialize",QMessageBox::Ok);
        _plpCore->release(fileReader);
        _plpCore->release(indexReader);
        _plpCore->release(indexWriter);
        if(comparator){
            delete comparator;
        }
        return;
    }



    std::function<void(int, unsigned long long)> update = [&](int percent, unsigned long long numResults){
        emit progressUpdate(percent, numResults);
    };

    if(!_progressDialog){
        _progressDialog = new QProgressDialog("", "Cancel", 0, 100, this);
        _progressDialog->setWindowTitle("Searching...");
        _progressDialog->setWindowModality(Qt::WindowModal);
        _progressDialog->setMinimumWidth(400);
    }
    _progressDialog->show();

    PLP::CoreI* core = _plpCore;
    QtConcurrent::run([this, core, fileReader, indexReader, indexWriter,
                      startLine, endLine, maxNumResults, comparator, update](){
       if(!core->search(fileReader, indexReader, indexWriter,
                    startLine, endLine, maxNumResults,comparator,&update)){
            emit searchError();
       }
       core->release(fileReader);
       core->release(indexReader);
       core->release(indexWriter);
       delete comparator;
    });
}

void SearchView::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    _filePath->setText(path);
}

void SearchView::openIndex(){
    QString path = QFileDialog::getOpenFileName(this, tr("Select indices to open")/*, "", tr("Index (*.plpidx)")*/);
    if(path.isEmpty()){
        return;
    }

    _indexPath->setText(path);
}

void SearchView::onProgressUpdate(int percent, unsigned long long numResults) {
    _progressDialog->setValue(percent);
    _progressDialog->setLabelText("Results: " + QString::number(numResults));
}

void SearchView::onSearchError() {
    _progressDialog->reset();
    _progressDialog->hide();

    QMessageBox::critical(this,"PLP","Search failed",QMessageBox::Ok);
}
