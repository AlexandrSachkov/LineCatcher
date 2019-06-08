#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchview.h"
#include "scriptview.h"
#include "aboutdialog.h"
#include "scriptdocsdialog.h"
#include "gettingstarteddialog.h"

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
    void openIndex(const QString& path);

signals:

public slots:
    void closeTab(int index);
    void closeAllTabs();
    void openFile();
    void openIndex();
    void showScriptView();
    void showStandardSearch();
    void showAdvancedSearch();
    void showAboutDialog();
    void showScriptingDocsDialog();
    void showGettingStartedDialog();
    void onDialogCancel();
    void exit();

private:
    bool openFile(const std::vector<QString>& candidates);
    bool openFile(const QString& path);

    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;
    QAction* _openFile;
    QAction* _exit;

    QTabWidget* _fileViewer;

    SearchView* _standardSearchView;
    SearchView* _advancedSearchView;
    ScriptView* _scriptView;
    AboutDialog* _aboutDialog;
    ScriptDocsDialog* _scriptDocsDialog;
    GettingStartedDialog* _gettingStartedDialog;

    PLP::CoreI* _plpCore;
};

#endif // MAINWINDOW_H
