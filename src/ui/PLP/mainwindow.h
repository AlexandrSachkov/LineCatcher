#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "scriptview.h"

#include <QMainWindow>
#include <QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QMenuBar>

#include "Core.h"

#include <unordered_map>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void closeTab(int index);
    void openFile();
    void openIndex();
    void showScriptView();

private:
    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;
    QAction* _openFile;
    QAction* _exit;

    //Ui::MainWindow *_ui;
    QTabWidget* _fileViewer;
    QTabWidget* _resultSetViewer;
    ScriptView* _scriptView;

    PLP::CoreI* _plpCore;
};

#endif // MAINWINDOW_H
