#include "searchview.h"
#include "common.h"
#include "mainwindow.h"

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

#include "Utils.h"
#include <unordered_map>

SearchView::SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;
    _multiline = multiline;

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
    connect(this, SIGNAL(searchCompleted(bool)), this, SLOT(onSearchCompletion(bool)));

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

    QHBoxLayout* openDirLayout = new QHBoxLayout();
    _destDir = new QLineEdit(this);
    _destDir->setReadOnly(true);
    QPushButton* openDir = new QPushButton("Open", this);
    connect(openDir, SIGNAL(clicked(void)), this, SLOT(openDestinationDir(void)));
    openDir->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openDirLayout->addWidget(_destDir);
    openDirLayout->addWidget(openDir);
    destLayout->addRow("Directory: ", openDirLayout);

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

    _regex = new QCheckBox("Regex", this);
    searchLayout->addRow(_regex);

    _ignoreCase = new QCheckBox("Ignore case", this);
    searchLayout->addRow(_ignoreCase);
}

void SearchView::createMultilineSearchOptionContent(QLayout* mainLayout){
    QGroupBox* searchGroup = new QGroupBox("Search", this);
    searchGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(searchGroup);

    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(4);
    searchGroup->setLayout(formLayout);

    //Enabled
    {
        for(int i = 0; i < NUM_ROWS; i++){
            QCheckBox* lineEnabledCheckBox = new QCheckBox(this);
            _lineEnabledCheckBoxes.push_back(lineEnabledCheckBox);
            formLayout->addWidget(lineEnabledCheckBox, i + 1, 0);
        }
    }

    //Line offset
    {
        QLabel* lineOffsetLabel = new QLabel("Line #", this);
        lineOffsetLabel->setAlignment(Qt::AlignHCenter);
        formLayout->addWidget(lineOffsetLabel, 0, 1);

        for(int i = 0; i < NUM_ROWS; i++){
            QSpinBox* lineOffsetBox = new QSpinBox(this);
            lineOffsetBox->setRange(-10, 10);
            lineOffsetBox->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
            _lineOffsetBoxes.push_back(lineOffsetBox);
            formLayout->addWidget(lineOffsetBox, i + 1, 1);
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

    //Regex
    {
        for(int i = 0; i < NUM_ROWS; i++){
            QCheckBox* regexCheckBox = new QCheckBox("Regex", this);
            _regexCheckBoxes.push_back(regexCheckBox);
            formLayout->addWidget(regexCheckBox, i + 1, 3);
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
    QString destDir = _destDir->text();
    QString destName = _destName->text();
    unsigned long long startLine = _fromLineBox->value();
    unsigned long long endLine = _toLineBox->value();
    unsigned long long maxNumResults = _numResultsBox->value();

    //validate user input
    if(dataPath.simplified().isEmpty()){
        QMessageBox::information(this,"PLP","Source file path cannot be empty",QMessageBox::Ok);
        return;
    }

    if(destDir.simplified().isEmpty()){
        QMessageBox::information(this,"PLP","Save to file directory cannot be empty",QMessageBox::Ok);
        return;
    }

    if(destName.simplified().isEmpty()){
        QMessageBox::information(this,"PLP","Save to file name cannot be empty",QMessageBox::Ok);
        return;
    }

    if(startLine > endLine){
        QMessageBox::information(this,"PLP","Start line must be smaller or equal to end line",QMessageBox::Ok);
        return;
    }

    // Create required objects
    PLP::FileReaderI* fileReader = _plpCore->createFileReader(dataPath.toStdString(), 0, true, nullptr);
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

    QString destPath = destDir + "/" + destName;
    PLP::ResultSetWriterI* indexWriter = _plpCore->createResultSetWriter(destPath.toStdString(), 0, fileReader, true);
    if(!indexWriter){
        QMessageBox::information(this,"PLP","Index writer failed to initialize",QMessageBox::Ok);
        _plpCore->release(fileReader);
        _plpCore->release(indexReader);
        return;
    }

    if(_multiline){
        startMultilineSearch(fileReader, indexReader, indexWriter, startLine, endLine, maxNumResults);
    }else{
        startRegularSearch(fileReader, indexReader, indexWriter, startLine, endLine, maxNumResults);
    }
}

void SearchView::startRegularSearch(
        PLP::FileReaderI* fileReader,
        PLP::ResultSetReaderI* indexReader,
        PLP::ResultSetWriterI* indexWriter,
        unsigned long long startLine,
        unsigned long long endLine,
        unsigned long long maxNumResults
        ){
    QString searchPattern = _searchField->text();
    if(searchPattern.isEmpty()){
        QMessageBox::information(this,"PLP","Search pattern cannot be empty",QMessageBox::Ok);
        return;
    }

    bool regex = _regex->isChecked();
    bool ignoreCase = _ignoreCase->isChecked();

    PLP::TextComparator* comparator = nullptr;
    if(regex){
        comparator = new PLP::MatchRegex(searchPattern.toStdString(), ignoreCase);
    }else{
        comparator = new PLP::MatchString(searchPattern.toStdString(), false, ignoreCase);
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
        connect(_progressDialog, SIGNAL(canceled(void)), this, SLOT(onSearchCancelled(void)));
    }
    _progressDialog->show();

    PLP::CoreI* core = _plpCore;
    QtConcurrent::run([this, core, fileReader, indexReader, indexWriter,
                      startLine, endLine, maxNumResults, comparator, update](){
       bool result = core->search(fileReader, indexReader, indexWriter,
                    startLine, endLine, maxNumResults,comparator,&update);

       core->release(fileReader);
       core->release(indexReader);
       core->release(indexWriter);
       delete comparator;

       emit searchCompleted(result);
    });
}

void SearchView::startMultilineSearch(
        PLP::FileReaderI* fileReader,
        PLP::ResultSetReaderI* indexReader,
        PLP::ResultSetWriterI* indexWriter,
        unsigned long long startLine,
        unsigned long long endLine,
        unsigned long long maxNumResults
        ){

    bool error = false;
    std::unordered_map<int, PLP::TextComparator*> lineComparators;
    for(int i=0; i < NUM_ROWS; i++){
        if(_lineEnabledCheckBoxes[i]->isChecked()){
            if(_searchPatternBoxes[i]->text().isEmpty()){
                QMessageBox::information(
                    this,
                    "PLP",
                    "Line #" + QString::number(_lineOffsetBoxes[i]->value()) + " cannot be empty",
                    QMessageBox::Ok
                );
                error = true;
                break;
            }

            if(_regexCheckBoxes[i]->isChecked()){
                PLP::TextComparator* comparator = new PLP::MatchRegex(
                    _searchPatternBoxes[i]->text().toStdString(),
                    _ignoreCaseCheckBoxes[i]->isChecked()
                );
                lineComparators.emplace(_lineOffsetBoxes[i]->value(), comparator);
            }else{
                PLP::TextComparator* comparator = new PLP::MatchString(
                    _searchPatternBoxes[i]->text().toStdString(),
                    false,
                    _ignoreCaseCheckBoxes[i]->isChecked()
                );
                lineComparators.emplace(_lineOffsetBoxes[i]->value(), comparator);
            }
        }
    }

    std::function<void()> cleanup = [&](){
        _plpCore->release(indexWriter);
        _plpCore->release(indexReader);
        _plpCore->release(fileReader);

        for(auto& pair : lineComparators){
            delete pair.second;
        }
    };

    if(error){
        cleanup();
        return;
    }

    for(auto& pair : lineComparators){
        if(!pair.second->initialize()){
            QMessageBox::information(
                this,
                "PLP",
                "Failed to initialize comparator on Line #" + QString::number(pair.first),
                QMessageBox::Ok
            );
            error = true;
        }
    }

    if(error){
        cleanup();
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
        connect(_progressDialog, SIGNAL(canceled(void)), this, SLOT(onSearchCancelled(void)));
    }
    _progressDialog->show();

    PLP::CoreI* core = _plpCore;
    QtConcurrent::run([this, core, fileReader, indexReader, indexWriter,
                      startLine, endLine, maxNumResults, lineComparators, update, cleanup](){
       bool result = core->searchMultiline(fileReader, indexReader, indexWriter,
                    startLine, endLine, maxNumResults,lineComparators,&update);

       _plpCore->release(fileReader);
       _plpCore->release(indexReader);
       _plpCore->release(indexWriter);

       for(auto& pair : lineComparators){
           delete pair.second;
       }

       emit searchCompleted(result);
    });
}

void SearchView::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    _filePath->setText(path);
    if(_destDir->text().isEmpty()){
        _destDir->setText(Common::getDirFromPath(path));
    }
}

void SearchView::openIndex(){
    QString path = QFileDialog::getOpenFileName(this, tr("Select indices to open")/*, "", tr("Index (*.plpidx)")*/);
    if(path.isEmpty()){
        return;
    }

    _indexPath->setText(path);
}

void SearchView::openDestinationDir(){
    QString path = QFileDialog::getExistingDirectory(this, tr("Select directory"));
    if(path.isEmpty()){
        return;
    }

    _destDir->setText(path);
}

void SearchView::onProgressUpdate(int percent, unsigned long long numResults) {
    _progressDialog->setValue(percent);
    _progressDialog->setLabelText("Results: " + QString::number(numResults));
}

void SearchView::onSearchCompletion(bool success){
    _progressDialog->reset();
    _progressDialog->hide();

    if(_plpCore->isCancelled()){
        return;
    }

    if(!success){
        QMessageBox::critical(this,"PLP","Search failed",QMessageBox::Ok);
        return;
    }

    std::wstring fileNameWithExt =
            PLP::getFileNameNoExt(_destName->text().toStdWString()) + PLP::string_to_wstring(PLP::FILE_INDEX_EXTENSION);
    QString indexPath = _destDir->text() + "/" + QString::fromStdWString(fileNameWithExt);
    static_cast<MainWindow*>(parent())->openIndex(indexPath);
    this->hide();
}

void SearchView::onSearchCancelled(){
    _plpCore->cancelOperation();
}
