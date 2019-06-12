#include "searchview.h"
#include "common.h"
#include "mainwindow.h"
#include "coreobjptr.h"

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
#include <QApplication>
#include <QDesktopWidget>

#include "Utils.h"
#include <unordered_map>

SearchView::SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;
    _multiline = multiline;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    setWindowTitle("Search");
    setMinimumWidth(450);

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
    QPushButton* openFile = new QPushButton("Browse", this);
    connect(openFile, SIGNAL(clicked(void)), this, SLOT(openFile(void)));
    openFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openFileLayout->addWidget(_filePath);
    openFileLayout->addWidget(openFile);
    sourceLayout->addRow("File: ", openFileLayout);
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
    QPushButton* openDir = new QPushButton("Browse", this);
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

    QHBoxLayout* openIndexLayout = new QHBoxLayout();
    _indexPath = new QLineEdit(this);
    _indexPath->setReadOnly(true);
    QPushButton* openIndex = new QPushButton("Browse", this);
    connect(openIndex, SIGNAL(clicked(void)), this, SLOT(openIndex(void)));
    openIndex->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    openIndexLayout->addWidget(_indexPath);
    openIndexLayout->addWidget(openIndex);

    limiterLayout->addRow("Index (Optional): ", openIndexLayout);
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
        QMessageBox::information(this,"Error","Source file path cannot be empty",QMessageBox::Ok);
        return;
    }

    if(destDir.simplified().isEmpty()){
        QMessageBox::information(this,"Error","Save to file directory cannot be empty",QMessageBox::Ok);
        return;
    }

    if(destName.simplified().isEmpty()){
        QMessageBox::information(this,"Error","Save to file name cannot be empty",QMessageBox::Ok);
        return;
    }

    if(startLine > endLine){
        QMessageBox::information(this,"Error","Start line must be smaller or equal to end line",QMessageBox::Ok);
        return;
    }

    QProgressDialog dialog(this);
    dialog.setWindowTitle("Opening file...");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumWidth(400);

    QFutureWatcher<PLP::FileReaderI*> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<PLP::FileReaderI*>::finished, &dialog, &QProgressDialog::reset);
    QObject::connect(&dialog, &QProgressDialog::canceled, this, &SearchView::onSearchCancelled);

    futureWatcher.setFuture(QtConcurrent::run([&, dataPath]() -> PLP::FileReaderI* {
        std::function<void(int)> update = [&](int percent){
            QMetaObject::invokeMethod(&dialog, [&, percent](){
                dialog.setValue(percent);
            });
        };
        return _plpCore->createFileReader(dataPath.toStdString(), 0, nullptr);
    }));

    dialog.exec();
    futureWatcher.waitForFinished();

    // Create required objects
    CoreObjPtr<PLP::FileReaderI> fileReader = createCoreObjPtr(futureWatcher.result(), _plpCore);
    if(!fileReader){
        QMessageBox::information(this,"Error","File reader failed to initialize",QMessageBox::Ok);
        return;
    }


    CoreObjPtr<PLP::IndexReaderI> indexReader = nullptr;
    if(!indexPath.simplified().isEmpty()){
        indexReader = createCoreObjPtr(_plpCore->createIndexReader(indexPath.toStdString(), 0), _plpCore);
        if(!indexReader){
            QMessageBox::information(this,"Error","Index reader failed to initialize",QMessageBox::Ok);
            return;
        }
    }

    QString destPath = destDir + "/" + destName;
    CoreObjPtr<PLP::IndexWriterI> indexWriter = createCoreObjPtr(
        _plpCore->createIndexWriter(destPath.toStdString(), 0, fileReader.get(), true),
        _plpCore
    );
    if(!indexWriter){
        QMessageBox::information(this,"Error","Index writer failed to initialize",QMessageBox::Ok);
        return;
    }

    if(_multiline){
        startMultilineSearch(std::move(fileReader),
             std::move(indexReader),
             std::move(indexWriter),
             startLine, endLine, maxNumResults
        );
    }else{
        startRegularSearch(
            std::move(fileReader),
            std::move(indexReader),
            std::move(indexWriter),
            startLine, endLine, maxNumResults
        );
    }
}

void SearchView::startRegularSearch(
        CoreObjPtr<PLP::FileReaderI> fileReader,
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        CoreObjPtr<PLP::IndexWriterI> indexWriter,
        unsigned long long startLine,
        unsigned long long endLine,
        unsigned long long maxNumResults
        ){
    QString searchPattern = _searchField->text();
    if(searchPattern.isEmpty()){
        QMessageBox::information(this,"Error","Search pattern cannot be empty",QMessageBox::Ok);
        return;
    }

    bool regex = _regex->isChecked();
    bool ignoreCase = _ignoreCase->isChecked();

    std::unique_ptr<PLP::TextComparator> comparator = nullptr;
    if(regex){
        comparator.reset(new PLP::MatchRegex(searchPattern.toStdString(), ignoreCase));
    }else{
        comparator.reset(new PLP::MatchString(searchPattern.toStdString(), false, ignoreCase));
    }

    if(!comparator || !comparator->initialize()){
        QMessageBox::information(this,"Error","Comparator failed to initialize",QMessageBox::Ok);
        return;
    }


    QProgressDialog dialog(this);
    dialog.setWindowTitle("Searching...");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumWidth(400);

    QFutureWatcher<bool> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<PLP::FileReaderI*>::finished, &dialog, &QProgressDialog::reset);
    QObject::connect(&dialog, &QProgressDialog::canceled, this, &SearchView::onSearchCancelled);

    futureWatcher.setFuture(
        QtConcurrent::run([&, startLine, endLine, maxNumResults]() -> bool {
            std::function<void(int, unsigned long long)> update = [&](int percent, unsigned long long numResults){
                QMetaObject::invokeMethod(&dialog, [&, percent, numResults](){
                    dialog.setValue(percent);
                    dialog.setLabelText("Results: " + QString::number(numResults));
                });
            };
            return _plpCore->search(
                fileReader.get(), indexReader.get(), indexWriter.get(),
                startLine, endLine, maxNumResults,
                comparator.get(),
                &update
            );
        })
    );

    dialog.exec();
    futureWatcher.waitForFinished();

    fileReader.reset();
    indexReader.reset();
    indexWriter.reset();

    onSearchCompletion(futureWatcher.result());
}

void SearchView::startMultilineSearch(
        CoreObjPtr<PLP::FileReaderI> fileReader,
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        CoreObjPtr<PLP::IndexWriterI> indexWriter,
        unsigned long long startLine,
        unsigned long long endLine,
        unsigned long long maxNumResults
        ){

    std::unordered_map<int, std::shared_ptr<PLP::TextComparator>> lineComparators;
    for(size_t i=0; i < (size_t)NUM_ROWS; i++){
        if(_lineEnabledCheckBoxes[i]->isChecked()){
            if(_searchPatternBoxes[i]->text().isEmpty()){
                QMessageBox::information(
                    this,
                    "PLP",
                    "Line #" + QString::number(_lineOffsetBoxes[i]->value()) + " cannot be empty",
                    QMessageBox::Ok
                );
                return;
            }

            if(_regexCheckBoxes[i]->isChecked()){
                std::shared_ptr<PLP::TextComparator> comparator(new PLP::MatchRegex(
                    _searchPatternBoxes[i]->text().toStdString(),
                    _ignoreCaseCheckBoxes[i]->isChecked()
                ));
                lineComparators.emplace(_lineOffsetBoxes[i]->value(), comparator);
            }else{
                std::shared_ptr<PLP::TextComparator> comparator(new PLP::MatchString(
                    _searchPatternBoxes[i]->text().toStdString(),
                    false,
                    _ignoreCaseCheckBoxes[i]->isChecked()
                ));
                lineComparators.emplace(_lineOffsetBoxes[i]->value(), comparator);
            }
        }
    }

    for(auto& pair : lineComparators){
        if(!pair.second->initialize()){
            QMessageBox::information(
                this,
                "PLP",
                "Failed to initialize comparator on Line #" + QString::number(pair.first),
                QMessageBox::Ok
            );
            return;
        }
    }

    QProgressDialog dialog(this);
    dialog.setWindowTitle("Searching...");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumWidth(400);

    QFutureWatcher<bool> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<PLP::FileReaderI*>::finished, &dialog, &QProgressDialog::reset);
    QObject::connect(&dialog, &QProgressDialog::canceled, this, &SearchView::onSearchCancelled);

    futureWatcher.setFuture(
        QtConcurrent::run([&, startLine, endLine, maxNumResults, lineComparators]() -> bool {
            std::function<void(int, unsigned long long)> update = [&](int percent, unsigned long long numResults){
                QMetaObject::invokeMethod(&dialog, [&, percent, numResults](){
                    dialog.setValue(percent);
                    dialog.setLabelText("Results: " + QString::number(numResults));
                });
            };

            std::unordered_map<int, PLP::TextComparator*> comparators;
            for(auto& comp : lineComparators){
                comparators.emplace(comp.first, comp.second.get());
            }
            return _plpCore->searchMultiline(
                fileReader.get(), indexReader.get(), indexWriter.get(),
                startLine, endLine, maxNumResults,
                comparators,
                &update
            );
        })
    );

    dialog.exec();
    futureWatcher.waitForFinished();

    fileReader.reset();
    indexReader.reset();
    indexWriter.reset();

    onSearchCompletion(futureWatcher.result());
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
    std::string fileFilter = "Index (*" + std::string(PLP::FILE_INDEX_EXTENSION) +")";
    QString path = QFileDialog::getOpenFileName(this, tr("Select indices to open"), "",
        tr(fileFilter.c_str()));

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

void SearchView::onSearchCompletion(bool success){
    if(_plpCore->isCancelled()){
        return;
    }

    if(!success){
        QMessageBox::critical(this,"Error","Search failed",QMessageBox::Ok);
        return;
    }

    std::wstring fileName = _destName->text().toStdWString();
    if (std::wstring::npos == fileName.find(PLP::string_to_wstring(PLP::FILE_INDEX_EXTENSION))) {
        fileName += PLP::string_to_wstring(PLP::FILE_INDEX_EXTENSION);
    }

    QString indexPath = _destDir->text() + "/" + QString::fromStdWString(fileName);
    static_cast<MainWindow*>(parent())->openIndex(indexPath);
    this->hide();
}

void SearchView::onSearchCancelled(){
    _plpCore->cancelOperation();
}

void SearchView::showEvent(QShowEvent* event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int defaultPosX = (screenGeometry.width() - size().width()) / 2;
    int defaultPosY = (screenGeometry.height() - size().height()) / 2;
    move(defaultPosX, defaultPosY);
}
