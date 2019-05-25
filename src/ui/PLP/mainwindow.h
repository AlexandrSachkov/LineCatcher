#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchview.h"
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
    bool openFile(const std::vector<QString>& candidates);
    bool openFile(const QString& path);
    void openIndex(const QString& path);
    QString getDirFromPath(const QString& path);

    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;
    QAction* _openFile;
    QAction* _exit;

    QTabWidget* _fileViewer;
    QTabWidget* _resultSetViewer;

    SearchView* _standardSearchView;
    SearchView* _advancedSearchView;
    ScriptView* _scriptView;

    PLP::CoreI* _plpCore;
};

#endif // MAINWINDOW_H
