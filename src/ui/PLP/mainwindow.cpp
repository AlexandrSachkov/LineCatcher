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

#include "Utils.h"
#include "IndexReaderI.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Programmable Log Parser");
    _centralWidget = new QWidget();
    setCentralWidget(_centralWidget);

    _plpCore = PLP::createCore();
    if(!_plpCore || !_plpCore->initialize()){
        QMessageBox::critical(this,"PLP","Application failed to initialize and needs to exit.",QMessageBox::Ok);
        PLP::release(_plpCore);
        exit(1);
    }

    _mainLayout = new QVBoxLayout(_centralWidget);
    _centralWidget->setLayout(_mainLayout);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(2, 2, 2, 2);

    QMenu* fileMenu = new QMenu("File");
    menuBar()->addMenu(fileMenu);

    QMenu* openMenu = new QMenu("Open");
    fileMenu->addMenu(openMenu);

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

    QMenu* configMenu = new QMenu("Config");
    menuBar()->addMenu(configMenu);

    QMenu* helpMenu = new QMenu("Help");
    menuBar()->addMenu(helpMenu);

    QAction* about = new QAction("About", helpMenu);
    helpMenu->addAction(about);
    connect(about, SIGNAL(triggered(void)), this, SLOT(showAboutDialog(void)));

    _fileViewer = new QTabWidget(_centralWidget);
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
        QMessageBox::critical(this,"PLP","Failed to open file: " + path,QMessageBox::Ok);
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

    QProgressDialog dialog(this);
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

    if(_plpCore->isCancelled()){
        return false;
    }

    PLP::FileReaderI* fileReader = futureWatcher.result();
    if(!fileReader){
        return false;
    }

    CoreObjPtr<PLP::FileReaderI> fileReaderCP(
        fileReader,
        [&](PLP::FileReaderI* p){
            _plpCore->release(p);
        }
    );

    FileView* fileView = new FileView(std::move(fileReaderCP), this);
    QString fileName = path.split('/').last();
    _fileViewer->addTab(fileView, fileName);
    _fileViewer->setTabToolTip(_fileViewer->count() - 1, path);
    _fileViewer->setCurrentIndex(_fileViewer->count() - 1);
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
    CoreObjPtr<PLP::IndexReaderI> indexReader(
        _plpCore->createIndexReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2),
        [&](PLP::IndexReaderI* p){
            _plpCore->release(p);
        }
    );
    if(!indexReader){
        QMessageBox::critical(this,"PLP","Failed to open index: " + path,QMessageBox::Ok);
        return;
    }

    QString qDataFileOriginalPath = QString::fromStdString(indexReader->getDataFilePath());
    QString qDataFileLocalPath = Common::getDirFromPath(path) + path.split('/').last();
    std::vector<QString> possibleFileLocations({qDataFileOriginalPath, qDataFileLocalPath});
    if(!openFile(possibleFileLocations)){
        if(!_plpCore->isCancelled()){
            QMessageBox::critical(this,
                "PLP","Failed to open file at " + qDataFileOriginalPath + " or " + qDataFileLocalPath,
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
                return;
            }
        }
    }

    QMessageBox::critical(this,"PLP","Failed to open index: " + path,QMessageBox::Ok);
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
