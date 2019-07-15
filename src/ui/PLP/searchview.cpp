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
#include "CoreI.h"
#include "FileReaderI.h"
#include "IndexReaderI.h"
#include <unordered_map>

SearchView::SearchView(PLP::CoreI* plpCore, bool multiline, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;
    _multiline = multiline;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
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

    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");
    QString searchDestDir = settings.value("searchDestDir", settings.value("fileOpenDir", "")).toString();
    settings.endGroup();

    QHBoxLayout* openDirLayout = new QHBoxLayout();
    _destDir = new QLineEdit(this);
    _destDir->setReadOnly(true);
    _destDir->setText(searchDestDir);
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

        for(int i = 0, val = - (NUM_ROWS / 2); i < NUM_ROWS; i++, val++){
            QSpinBox* lineOffsetBox = new QSpinBox(this);
            lineOffsetBox->setRange(-10, 10);
            lineOffsetBox->setValue(val);
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

    if(endLine != 0 && startLine > endLine){
        QMessageBox::information(this,"Error","Start line must be smaller or equal to end line",QMessageBox::Ok);
        return;
    }

    QProgressDialog dialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
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
        return _plpCore->createFileReader(dataPath.toStdString(), 0, &update);
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

    if(indexReader && QString::fromUtf8(indexReader->getDataFilePath()) != dataPath){
        QMessageBox::information(this,"Error","Index file must correspond to the source file",QMessageBox::Ok);
        return;
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

    std::unique_ptr<PLP::TextComparator> comparator = nullptr;
    if(regex){
        comparator.reset(new PLP::MatchRegex(searchPattern.toStdString()));
    }else{
        comparator.reset(new PLP::MatchString(searchPattern.toStdString(), false));
    }

    if(!comparator || !comparator->initialize()){
        QMessageBox::information(this,"Error","Comparator failed to initialize",QMessageBox::Ok);
        return;
    }


    QProgressDialog dialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
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
                    _searchPatternBoxes[i]->text().toStdString()
                ));
                lineComparators.emplace(_lineOffsetBoxes[i]->value(), comparator);
            }else{
                std::shared_ptr<PLP::TextComparator> comparator(new PLP::MatchString(
                    _searchPatternBoxes[i]->text().toStdString(),
                    false
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

    QProgressDialog dialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
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
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");

    QString fileOpenDir = settings.value("fileOpenDir", "").toString();
    QString path = QFileDialog::getOpenFileName(this, "Select file to open", fileOpenDir);
    if(path.isEmpty()){
        settings.endGroup();
        return;
    }

    settings.setValue("fileOpenDir", Common::getDirFromPath(path));
    settings.endGroup();

    _filePath->setText(path);
    if(_destDir->text().isEmpty()){
        _destDir->setText(Common::getDirFromPath(path));
    }
}

void SearchView::openIndex(){
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");

    QString indexOpenDir = settings.value("indexOpenDir", settings.value("fileOpenDir", "")).toString();
    std::string fileFilter = "Index (*" + std::string(PLP::FILE_INDEX_EXTENSION) +")";
    QString path = QFileDialog::getOpenFileName(this, tr("Select index to open"), indexOpenDir,
        tr(fileFilter.c_str()));

    if(!path.trimmed().isEmpty()){
        settings.setValue("indexOpenDir", Common::getDirFromPath(path));
    }
    settings.endGroup();

    _indexPath->setText(path);
}

void SearchView::openDestinationDir(){
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");

    QString searchDestDir = settings.value("searchDestDir", settings.value("fileOpenDir", "")).toString();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), searchDestDir);
    if(dir.isEmpty()){
        settings.endGroup();
        return;
    }

    settings.setValue("searchDestDir", dir);
    settings.endGroup();

    _destDir->setText(dir);
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

void SearchView::setSourcePath(const QString& path){
    _filePath->setText(path);
}
