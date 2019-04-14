#include "standardsearchview.h"
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
    connect(runSearch, SIGNAL(clicked(void)), this, SLOT(startSearch(void)));
    mainLayout->addWidget(runSearch);

    connect(this, SIGNAL(progressUpdate(int, unsigned long long)), this, SLOT(onProgressUpdate(int, unsigned long long)));
    connect(this, SIGNAL(searchError(void)), this, SLOT(onSearchError(void)));

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

void StandardSearchView::createDestinationContent(QLayout* mainLayout){
    QGroupBox* destGroup = new QGroupBox("Save results to", this);
    destGroup->setStyleSheet(SEARCH_GROUP_STYLESHEET);
    mainLayout->addWidget(destGroup);

    //Open file
    QFormLayout* destLayout = new QFormLayout();
    destGroup->setLayout(destLayout);

    _destName = new QLineEdit(this);
    destLayout->addRow("File name: ", _destName);
}

void StandardSearchView::createSearchLimiterContent(QLayout* mainLayout){
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

void StandardSearchView::createSearchOptionContent(QLayout* mainLayout) {
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

void StandardSearchView::startSearch() {
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

void StandardSearchView::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    _filePath->setText(path);
}

void StandardSearchView::openIndex(){
    QString path = QFileDialog::getOpenFileName(this, tr("Select indices to open")/*, "", tr("Index (*.plpidx)")*/);
    if(path.isEmpty()){
        return;
    }

    _indexPath->setText(path);
}

void StandardSearchView::onProgressUpdate(int percent, unsigned long long numResults) {
    _progressDialog->setValue(percent);
    _progressDialog->setLabelText("Results: " + QString::number(numResults));
}

void StandardSearchView::onSearchError() {
    _progressDialog->reset();
    _progressDialog->hide();

    QMessageBox::critical(this,"PLP","Search failed",QMessageBox::Ok);
}
