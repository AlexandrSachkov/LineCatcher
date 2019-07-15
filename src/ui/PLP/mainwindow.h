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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "searchview.h"
#include "scriptview.h"
#include "aboutdialog.h"
#include "scriptdocsdialog.h"
#include "gettingstarteddialog.h"
#include "settingsdialog.h"

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
    void showSettingsDialog();
    void onDialogCancel();
    void exit();

private:
    bool openFile(const std::vector<QString>& candidates);
    bool openFile(const QString& path);
    void setFontSize(int pointSize);
    void closeEvent(QCloseEvent *event);

    int _viewerFontSize = 12;
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
    SettingsDialog* _settingsDialog;

    PLP::CoreI* _plpCore;
};

#endif // MAINWINDOW_H
