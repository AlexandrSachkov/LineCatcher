#include "mainwindow.h"

#include "fileview.h"
#include <QTabBar>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>

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

    QMenu* configMenu = new QMenu("Config", _menuBar);
    _menuBar->addMenu(configMenu);

    QMenu* helpMenu = new QMenu("Help", _menuBar);
    _menuBar->addMenu(helpMenu);

    _fileViewer = new QTabWidget(_centralWidget);
    _mainLayout->addWidget(_fileViewer);
    _fileViewer->setTabsClosable(true);
    connect(_fileViewer, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

MainWindow::~MainWindow() {
    //delete _ui;
}

void MainWindow::closeTab(int index) {
    if (index == -1) {
        return;
    }

    _fileViewer->removeTab(index);
}

void MainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    FileView* fileView = new FileView(path);
    QString fileName = path.split('/').last();
    _fileViewer->addTab(fileView, fileName);
    fileView->show();
}
