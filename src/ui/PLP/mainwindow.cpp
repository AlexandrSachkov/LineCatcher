#include "mainwindow.h"

#include "fileview.h"
#include <QTabBar>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QSplitter>
#include <QMessageBox>

#include "Utils.h"
#include "ResultSetReaderI.h"
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)//,
    //_ui(new Ui::MainWindow)
{
    setWindowTitle("Programmable Log Parser");
    _centralWidget = new QWidget();
    setCentralWidget(_centralWidget);

    _mainLayout = new QVBoxLayout(_centralWidget);
    _centralWidget->setLayout(_mainLayout);
    _mainLayout->setSpacing(6);
    _mainLayout->setContentsMargins(2, 25, 2, 2);

    _menuBar = new QMenuBar(this);
    _menuBar->setGeometry(QRect(0, 0, 1043, 21));

    QMenu* fileMenu = new QMenu("File", _menuBar);
    _menuBar->addMenu(fileMenu);

    QMenu* openMenu = new QMenu("Open", _menuBar);
    fileMenu->addMenu(openMenu);

    _openFile = new QAction("Data", openMenu);
    openMenu->addAction(_openFile);
    connect(_openFile, SIGNAL(triggered(void)), this, SLOT(openFile(void)));

    QAction* openIndex = new QAction("Index", openMenu);
    openMenu->addAction(openIndex);
    connect(openIndex, SIGNAL(triggered(void)), this, SLOT(openIndex(void)));

    QMenu* runMenu = new QMenu("Run", _menuBar);
    _menuBar->addMenu(runMenu);

    QAction* runScript = new QAction("Script", _menuBar);
    runMenu->addAction(runScript);
    connect(runScript, SIGNAL(triggered(void)), this, SLOT(showScriptView(void)));

    QMenu* configMenu = new QMenu("Config", _menuBar);
    _menuBar->addMenu(configMenu);

    QMenu* helpMenu = new QMenu("Help", _menuBar);
    _menuBar->addMenu(helpMenu);

    _fileViewer = new QTabWidget(_centralWidget);
    _mainLayout->addWidget(_fileViewer);
    _fileViewer->setTabsClosable(true);
    connect(_fileViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    _plpCore = PLP::createCore();
    if(_plpCore && !_plpCore->initialize()){
        bool zz = true;
        //TODO
    }

    _scriptView = new ScriptView(_plpCore, this);
    _scriptView->hide();
}

MainWindow::~MainWindow() {
    //delete _ui;
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

void MainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    /*int numTabs = _fileViewer->count();
    for(int i = 0; i < numTabs; i++){
        FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
        const QString& existingPath = fileView->getFilePath();
        if(path.compare(existingPath) == 0){
            _fileViewer->setCurrentIndex(i);
            return;
        }
    }*/

    std::unique_ptr<PLP::FileReaderI> fileReader =
            std::unique_ptr<PLP::FileReaderI>(_plpCore->createFileReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2, true));
    if(!fileReader){
        return;
    }

    FileView* fileView = new FileView(std::move(fileReader), this);
    QString fileName = path.split('/').last();
    _fileViewer->addTab(fileView, fileName);
    _fileViewer->setTabToolTip(_fileViewer->count() - 1, path);
    _fileViewer->setCurrentIndex(_fileViewer->count() - 1);
    fileView->show();
}

void MainWindow::openIndex() {
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Select indices to open")/*, "", tr("Index (*.plpidx)")*/);
    for(QString path : paths){
        std::unique_ptr<PLP::ResultSetReaderI> indexReader =
                std::unique_ptr<PLP::ResultSetReaderI>(_plpCore->createResultSetReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2));
        if(!indexReader){
            return;
        }

        std::wstring dataPath;
        indexReader->getDataFilePath(dataPath);
        QString qDataPath = QString::fromStdWString(dataPath);

        int numTabs = _fileViewer->count();
        for(int i = 0; i < numTabs; i++){
            FileView* fileView = static_cast<FileView*>(_fileViewer->widget(i));
            const QString& existingPath = fileView->getFilePath();
            if(qDataPath.compare(existingPath) == 0){
                _fileViewer->setCurrentIndex(i);
                fileView->openIndex(std::move(indexReader));
                return;
            }
        }
    }
}

void MainWindow::showScriptView() {
    _scriptView->show();
}
