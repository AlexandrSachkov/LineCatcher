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

#include "mainwindow.h"

#include "fileview.h"
#include "coreobjptr.h"
#include "common.h"
#include <QTabBar>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QSplitter>
#include <QMessageBox>
#include <QFutureWatcher>
#include <QApplication>
#include <QSettings>

#include "Utils.h"
#include "IndexReaderI.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Line Catcher");
    _centralWidget = new QWidget();
    setCentralWidget(_centralWidget);

    _plpCore = PLP::createCore();
    if(!_plpCore || !_plpCore->initialize()){
        QMessageBox::critical(this,"Error","Application failed to initialize and needs to exit.",QMessageBox::Ok);
        PLP::release(_plpCore);
        QApplication::quit();
    }

    _mainLayout = new QVBoxLayout(_centralWidget);
    _centralWidget->setLayout(_mainLayout);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(2, 2, 2, 2);

    QFont menuFont = menuBar()->font();
    menuFont.setPointSize(12);
    menuBar()->setFont(menuFont);

    QMenu* fileMenu = new QMenu("File");
    menuBar()->addMenu(fileMenu);

    QMenu* openMenu = new QMenu("Open", fileMenu);
    fileMenu->addMenu(openMenu);

    QAction* closeData = new QAction("Close Data", fileMenu);
    closeData->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_X));
    closeData->setShortcutVisibleInContextMenu(true);
    fileMenu->addAction(closeData);
    connect(closeData, SIGNAL(triggered(void)), this, SLOT(closeCurrentData(void)));

    QAction* closeIndex = new QAction("Close Index", fileMenu);
    closeIndex->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
    closeIndex->setShortcutVisibleInContextMenu(true);
    fileMenu->addAction(closeIndex);
    connect(closeIndex, SIGNAL(triggered(void)), this, SLOT(closeCurrentIndex(void)));

    QAction* closeAll = new QAction("Close All", fileMenu);
    closeAll->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Escape));
    closeAll->setShortcutVisibleInContextMenu(true);
    fileMenu->addAction(closeAll);
    connect(closeAll, SIGNAL(triggered(void)), this, SLOT(closeAllTabs(void)));

    QAction* exit = new QAction("Exit", fileMenu);
    exit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F4));
    exit->setShortcutVisibleInContextMenu(true);
    fileMenu->addAction(exit);
    connect(exit, SIGNAL(triggered(void)), this, SLOT(exit(void)));

    QAction* openFile = new QAction("Data", openMenu);
    openFile->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    openFile->setShortcutVisibleInContextMenu(true);
    openMenu->addAction(openFile);
    connect(openFile, SIGNAL(triggered(void)), this, SLOT(openFile(void)));

    QAction* openIndex = new QAction("Index", openMenu);
    openIndex->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    openIndex->setShortcutVisibleInContextMenu(true);
    openMenu->addAction(openIndex);
    connect(openIndex, SIGNAL(triggered(void)), this, SLOT(openIndex(void)));

    QMenu* runMenu = new QMenu("Run");
    menuBar()->addMenu(runMenu);

    QAction* runStandardSearch = new QAction("Search", runMenu);
    runStandardSearch->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F));
    runStandardSearch->setShortcutVisibleInContextMenu(true);
    runMenu->addAction(runStandardSearch);
    connect(runStandardSearch, SIGNAL(triggered(void)), this, SLOT(showStandardSearch(void)));

    QAction* runAdvancedSearch = new QAction("Search (Advanced)", runMenu);
    runAdvancedSearch->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    runAdvancedSearch->setShortcutVisibleInContextMenu(true);
    runMenu->addAction(runAdvancedSearch);
    connect(runAdvancedSearch, SIGNAL(triggered(void)), this, SLOT(showAdvancedSearch(void)));

    QAction* runScript = new QAction("Script");
    runScript->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    runScript->setShortcutVisibleInContextMenu(true);
    runMenu->addAction(runScript);
    connect(runScript, SIGNAL(triggered(void)), this, SLOT(showScriptView(void)));

    QMenu* configMenu = new QMenu("Configure");
    menuBar()->addMenu(configMenu);

    QAction* settings = new QAction("Settings");
    configMenu->addAction(settings);
    connect(settings, SIGNAL(triggered(void)), this, SLOT(showSettingsDialog(void)));

    QMenu* helpMenu = new QMenu("Help", this);
    menuBar()->addMenu(helpMenu);

    QAction* gettingStarted = new QAction("Getting Started", helpMenu);
    gettingStarted->setShortcut(QKeySequence(Qt::Key_F1));
    gettingStarted->setShortcutVisibleInContextMenu(true);
    helpMenu->addAction(gettingStarted);
    connect(gettingStarted, SIGNAL(triggered(void)), this, SLOT(showGettingStartedDialog(void)));

    QAction* scriptingDocs = new QAction("Lua API Docs", helpMenu);
    scriptingDocs->setShortcut(QKeySequence(Qt::Key_F2));
    scriptingDocs->setShortcutVisibleInContextMenu(true);
    helpMenu->addAction(scriptingDocs);
    connect(scriptingDocs, SIGNAL(triggered(void)), this, SLOT(showScriptingDocsDialog(void)));

    QAction* about = new QAction("About", helpMenu);
    helpMenu->addAction(about);
    connect(about, SIGNAL(triggered(void)), this, SLOT(showAboutDialog(void)));

    QFont tabFont = this->font();
    tabFont.setPointSize(10);

    _fileViewer = new QTabWidget(_centralWidget);
    _fileViewer->setFont(tabFont);
    _mainLayout->addWidget(_fileViewer);
    _fileViewer->setTabsClosable(true);
    connect(_fileViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    _standardSearchView = new SearchView(_plpCore, false, this);
    _standardSearchView->hide();

    _advancedSearchView = new SearchView(_plpCore, true, this);
    _advancedSearchView->hide();

    _scriptView = new ScriptView(_plpCore, this);
    _scriptView->hide();

    _aboutDialog = new AboutDialog(this);
    _aboutDialog->hide();

    _scriptDocsDialog = new ScriptDocsDialog(this);
    _scriptDocsDialog->hide();

    _gettingStartedDialog = new GettingStartedDialog(this);
    _gettingStartedDialog->hide();

    _settingsDialog = new SettingsDialog(_plpCore, [&](int pointSize){
        setFontSize(pointSize);
    }, this);
    _settingsDialog->hide();
}

MainWindow::~MainWindow() {
    delete _fileViewer;
    delete _standardSearchView;
    delete _advancedSearchView;
    delete _scriptView;

    PLP::release(_plpCore);
}

void MainWindow::closeTab(int index) {
    if (index == -1) {
        return;
    }

    QWidget* tab = _fileViewer->widget(index);
    if(tab){
        delete tab;
    }
}

bool MainWindow::openFile(const std::vector<QString>& candidates){
    for(auto& candidate : candidates){
        if(openFile(candidate)){
            return true;
        }
    }
    return false;
}

void MainWindow::openFile() {
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");

    QString fileOpenDir = settings.value("fileOpenDir", "").toString();
    QString path = QFileDialog::getOpenFileName(this, "Select file to open", fileOpenDir).trimmed();
    if(path.isEmpty()){
        settings.endGroup();
        return;
    }

    settings.setValue("fileOpenDir", Common::getDirFromPath(path));
    settings.endGroup();

    if(!openFile(path) && !_plpCore->isCancelled()){
        QMessageBox::critical(this,
          "Error","Failed to open file: " + path +
          "\nEnsure that directory is writable for index generation",
          QMessageBox::Ok
        );
    }
}

bool MainWindow::openFile(const QString& path)
{
    int numTabs = _fileViewer->count();
    for(int i = 0; i < numTabs; i++){
        FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
        const QString& existingPath = fileView->getFilePath();
        if(path.compare(existingPath) == 0){
            _fileViewer->setCurrentIndex(i);
            return true;
        }
    }

    QProgressDialog dialog(this, Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    dialog.setWindowTitle("Opening file...");
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumWidth(400);

    QFutureWatcher<PLP::FileReaderI*> futureWatcher;
    QObject::connect(&futureWatcher, &QFutureWatcher<PLP::FileReaderI*>::finished, &dialog, &QProgressDialog::reset);
    QObject::connect(&dialog, &QProgressDialog::canceled, this, &MainWindow::onDialogCancel);

    PLP::CoreI* core = _plpCore;
    futureWatcher.setFuture(QtConcurrent::run([&, this, core, path]() -> PLP::FileReaderI* {
        std::function<void(int)> update = [&](int percent){
            QMetaObject::invokeMethod(&dialog, [&, percent](){
                dialog.setValue(percent);
            });
        };
        return core->createFileReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2, &update);
    }));

    dialog.exec();
    futureWatcher.waitForFinished();

    CoreObjPtr<PLP::FileReaderI> fileReader = createCoreObjPtr(futureWatcher.result(), _plpCore);
    if(_plpCore->isCancelled() || !fileReader){
        return false;
    }

    FileView* fileView = new FileView(std::move(fileReader), this);
    QString fileName = path.split('/').last();
    _fileViewer->addTab(fileView, fileName);
    _fileViewer->setTabToolTip(_fileViewer->count() - 1, path);
    _fileViewer->setCurrentIndex(_fileViewer->count() - 1);
    setFontSize(_viewerFontSize);
    fileView->show();

    return true;
}

void MainWindow::openIndex() {
    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("CommonDirectories");

    QString indexOpenDir = settings.value("indexOpenDir", settings.value("fileOpenDir", "")).toString();
    std::string fileFilter = "Index (*" + std::string(PLP::FILE_INDEX_EXTENSION) +")";
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Select indexes to open"), indexOpenDir,
        tr(fileFilter.c_str()));

    if(paths.empty()){
        settings.endGroup();
        return;
    }

    settings.setValue("indexOpenDir", Common::getDirFromPath(paths[0]));
    settings.endGroup();

    for(QString path : paths){
        openIndex(path);
    }
}

void MainWindow::openIndex(const QString& path, const QString& highlightPattern, bool regex) {
    CoreObjPtr<PLP::IndexReaderI> indexReader = createCoreObjPtr(
        _plpCore->createIndexReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2),
        _plpCore
    );

    if(!indexReader){
        QMessageBox::critical(this,"Error","Failed to open index: " + path,QMessageBox::Ok);
        return;
    }

    QString qDataFileOriginalPath = QString::fromStdString(indexReader->getDataFilePath());
    QString qDataFileLocalPath = Common::getDirFromPath(path) + "/" + qDataFileOriginalPath.split('/').last();
    std::vector<QString> possibleFileLocations({qDataFileOriginalPath, qDataFileLocalPath});
    if(!openFile(possibleFileLocations)){
        if(!_plpCore->isCancelled()){
            QMessageBox::critical(this,
                "Error","Failed to find file at locations " + qDataFileOriginalPath + " and " + qDataFileLocalPath,
                QMessageBox::Ok
            );
        }
        return;
    }

    for(auto& possibleLocation : possibleFileLocations){
        int numTabs = _fileViewer->count();
        for(int i = 0; i < numTabs; i++){
            FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
            const QString& existingPath = fileView->getFilePath();
            if(possibleLocation.compare(existingPath) == 0){
                _fileViewer->setCurrentIndex(i);
                fileView->openIndex(std::move(indexReader), highlightPattern, regex);
                setFontSize(_viewerFontSize);
                return;
            }
        }
    }

    QMessageBox::critical(this,"Error","Failed to open index: " + path,QMessageBox::Ok);
}

void MainWindow::openIndex(const QString& path){
    openIndex(path, "", false);
}

void MainWindow::showScriptView() {
    _scriptView->show();
}

void MainWindow::showStandardSearch() {
    FileView* fileView = static_cast<FileView*>(_fileViewer->currentWidget());
    if(fileView){
        _standardSearchView->setSourcePath(fileView->getFilePath());
    }
    _standardSearchView->show();
}

void MainWindow::showAdvancedSearch() {
    FileView* fileView = static_cast<FileView*>(_fileViewer->currentWidget());
    if(fileView){
        _advancedSearchView->setSourcePath(fileView->getFilePath());
    }
    _advancedSearchView->show();
}

void MainWindow::onDialogCancel(){
    _plpCore->cancelOperation();
}

void MainWindow::showAboutDialog(){
    _aboutDialog->show();
}

void MainWindow::showScriptingDocsDialog() {
    _scriptDocsDialog->show();
}

void MainWindow::showGettingStartedDialog() {
    _gettingStartedDialog->show();
}

void MainWindow::showSettingsDialog() {
    _settingsDialog->show();
}

void MainWindow::closeAllTabs(){
    for(int i = _fileViewer->count(); i >=0; i--){
        QWidget* tab = _fileViewer->widget(i);
        if(tab){
            delete tab;
        }
    }
}

void MainWindow::exit(){
    this->close();
}

void MainWindow::setFontSize(int pointSize) {
    _viewerFontSize = pointSize;

    for(int i = 0; i < _fileViewer->count(); i++){
        FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
        fileView->setFontSize(pointSize);
    }
    _scriptView->setFontSize(pointSize);
}

void MainWindow::closeEvent(QCloseEvent* event){
    if(_scriptView->hasUnsavedContent()){
        QMessageBox::StandardButton saveScriptDialog =
            QMessageBox::question(
                this,
                "",
                "Would you like to save script before exiting?",
                QMessageBox::Yes|QMessageBox::No
            );

        if (saveScriptDialog == QMessageBox::Yes && !_scriptView->saveScript()) {
            event->ignore();
            _scriptView->show();
            return;
        }
    }

    event->accept();
}

void MainWindow::closeCurrentData() {
    closeTab(_fileViewer->currentIndex());
}

void MainWindow::closeCurrentIndex() {
    FileView* fileView = static_cast<FileView*>(_fileViewer->currentWidget());
    if(fileView){
        fileView->closeCurrentIndex();
    }
}

void MainWindow::closeIndex(const QString& path) {
    for(int i = 0; i < _fileViewer->count(); i++){
        FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
        fileView->closeIndex(path);
    }
}
