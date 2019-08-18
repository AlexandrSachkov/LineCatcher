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

#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QLineEdit>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <QProgressBar>

#include "Core.h"
#include "scripteditor.h"

#include <functional>
#include <mutex>
#include <vector>

class ScriptView : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptView(PLP::CoreI* plpCore, QWidget *parent = nullptr);
    ~ScriptView();
    void setFontSize(int pointSize);
    bool hasUnsavedContent();

public slots:
    bool saveScript();

private slots:
    void openScript();

    void runScript();
    void clearConsole();
    void checkScriptCompleted();
    void saveScriptAs();

protected:
    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);

private:
    void setScriptModified(bool modified);
    void printLogDataToConsole();

    static const char* LOG_SUBSCRIBER_NAME;
    const int MAX_LINES_CONSOLE = 1000;
    std::function<void(int, const char*)> _appendLogData;
    std::mutex _logDataLock;
    std::vector<std::pair<int, QString>> _logData;

    bool _saved = false;

    QPushButton* _open;
    QPushButton* _save;
    QPushButton* _saveAs;
    QPushButton* _run;
    QPushButton* _clearConsole;
    QLineEdit* _scriptPath;
    ScriptEditor* _scriptEditor;
    QPlainTextEdit* _console;
    QProgressBar* _progressBar;
    std::unique_ptr<QFile> _file = nullptr;

    PLP::CoreI* _plpCore;
    QFuture<bool> _scriptResult;
    QTimer* _scriptRunTimer;
};

#endif // SCRIPTVIEW_H
