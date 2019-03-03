#include "mainwindow.h"

#include "fileview.h"
#include <QTabBar>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QSplitter>

#include "Utils.h"

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

    _openFile = new QAction("Open", _menuBar);
    fileMenu->addAction(_openFile);
    connect(_openFile, SIGNAL(triggered(void)), this, SLOT(openFile(void)));

    QMenu* runMenu = new QMenu("Run", _menuBar);
    _menuBar->addMenu(runMenu);

    QAction* runScript = new QAction("Script", _menuBar);
    runMenu->addAction(runScript);
    connect(runScript, SIGNAL(triggered(void)), this, SLOT(showScriptView(void)));

    QMenu* configMenu = new QMenu("Config", _menuBar);
    _menuBar->addMenu(configMenu);

    QMenu* helpMenu = new QMenu("Help", _menuBar);
    _menuBar->addMenu(helpMenu);

    QSplitter* splitter = new QSplitter(this);
    _mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(5);
    splitter->setChildrenCollapsible(false);

    _fileViewer = new QTabWidget(_centralWidget);
    splitter->addWidget(_fileViewer);
    _fileViewer->setTabsClosable(true);
    connect(_fileViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    _resultSetViewer = new QTabWidget(splitter);
    _resultSetViewer->setTabsClosable(true);
    splitter->addWidget(_resultSetViewer);

    _resultSetViewer->addTab(new QWidget(), "Tab test");

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
    _fileViewer->removeTab(index);
}

void MainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    std::unique_ptr<PLP::FileReaderI> fileReader =
            std::unique_ptr<PLP::FileReaderI>(_plpCore->createFileReader(path.toStdString(), PLP::OPTIMAL_BLOCK_SIZE_BYTES * 2, true));
    if(!fileReader){
        return;
    }

    FileView* fileView = new FileView(std::move(fileReader), this);
    QString fileName = path.split('/').last();
    _fileViewer->addTab(fileView, fileName);
    fileView->show();
}

void MainWindow::showScriptView() {
    _scriptView->show();
}
