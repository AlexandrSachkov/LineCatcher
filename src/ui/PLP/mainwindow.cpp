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

    QAction* closeAll = new QAction("Close All", fileMenu);
    fileMenu->addAction(closeAll);
    connect(closeAll, SIGNAL(triggered(void)), this, SLOT(closeAllTabs(void)));

    QAction* exit = new QAction("Exit", fileMenu);
    fileMenu->addAction(exit);
    connect(exit, SIGNAL(triggered(void)), this, SLOT(exit(void)));

    _openFile = new QAction("Data", openMenu);
    openMenu->addAction(_openFile);
    connect(_openFile, SIGNAL(triggered(void)), this, SLOT(openFile(void)));

    QAction* openIndex = new QAction("Index", openMenu);
    openMenu->addAction(openIndex);
    connect(openIndex, SIGNAL(triggered(void)), this, SLOT(openIndex(void)));

    QMenu* runMenu = new QMenu("Run");
    menuBar()->addMenu(runMenu);

    QAction* runStandardSearch = new QAction("Search", runMenu);
    runMenu->addAction(runStandardSearch);
    connect(runStandardSearch, SIGNAL(triggered(void)), this, SLOT(showStandardSearch(void)));

    QAction* runAdvancedSearch = new QAction("Search (Advanced)", runMenu);
    runMenu->addAction(runAdvancedSearch);
    connect(runAdvancedSearch, SIGNAL(triggered(void)), this, SLOT(showAdvancedSearch(void)));

    QAction* runScript = new QAction("Script");
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
    helpMenu->addAction(gettingStarted);
    connect(gettingStarted, SIGNAL(triggered(void)), this, SLOT(showGettingStartedDialog(void)));

    QAction* scriptingDocs = new QAction("Lua API Docs", helpMenu);
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

    _settingsDialog = new SettingsDialog([&](int pointSize){
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
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    if(!openFile(path) && !_plpCore->isCancelled()){
        QMessageBox::critical(this,"Error","Failed to open file: " + path,QMessageBox::Ok);
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
    std::string fileFilter = "Index (*" + std::string(PLP::FILE_INDEX_EXTENSION) +")";
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Select indices to open"), "",
        tr(fileFilter.c_str()));
    for(QString path : paths){
        openIndex(path);
    }
}

void MainWindow::openIndex(const QString& path){
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
                fileView->openIndex(std::move(indexReader));
                setFontSize(_viewerFontSize);
                return;
            }
        }
    }

    QMessageBox::critical(this,"Error","Failed to open index: " + path,QMessageBox::Ok);
}

void MainWindow::showScriptView() {
    _scriptView->show();
}

void MainWindow::showStandardSearch() {
    _standardSearchView->show();
}

void MainWindow::showAdvancedSearch() {
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
    QApplication::quit();
}

void MainWindow::setFontSize(int pointSize) {
    _viewerFontSize = pointSize;

    for(int i = 0; i < _fileViewer->count(); i++){
        FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
        fileView->setFontSize(pointSize);
    }
    _scriptView->setFontSize(pointSize);
}
