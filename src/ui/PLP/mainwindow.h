#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "standardsearchview.h"
#include "advancedsearchview.h"
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
    void showStandardSearch();
    void showAdvancedSearch();

private:
    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;
    QAction* _openFile;
    QAction* _exit;

    //Ui::MainWindow *_ui;
    QTabWidget* _fileViewer;
    QTabWidget* _resultSetViewer;

    StandardSearchView* _standardSearchView;
    AdvancedSearchView* _advancedSearchView;
    ScriptView* _scriptView;

    PLP::CoreI* _plpCore;
};

#endif // MAINWINDOW_H
