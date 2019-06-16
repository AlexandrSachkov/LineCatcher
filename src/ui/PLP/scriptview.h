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
signals:

private slots:
    void openScript();
    void saveScript();
    void runScript();
    void clearConsole();
    void onScriptModified();
    void checkScriptCompleted();

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
    QPushButton* _run;
    QPushButton* _clearConsole;
    QLineEdit* _scriptPath;
    ScriptEditor* _scriptEditor;
    QPlainTextEdit* _console;
    QProgressBar* _progressBar;

    PLP::CoreI* _plpCore;
    QFuture<bool> _scriptResult;
    QTimer* _scriptRunTimer;
};

#endif // SCRIPTVIEW_H
