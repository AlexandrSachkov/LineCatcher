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

#include <functional>
#include <mutex>
#include <vector>

class ScriptView : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptView(PLP::CoreI* plpCore, QWidget *parent = nullptr);
    ~ScriptView();
signals:

private slots:
    void openScript();
    void loadScript();
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
    std::function<void(int, const char*)> _appendLogData;
    std::mutex _logDataLock;
    std::vector<std::pair<int, QString>> _logData;

    bool _saved = false;

    QPushButton* _open;
    QPushButton* _load;
    QPushButton* _save;
    QPushButton* _run;
    QPushButton* _clearConsole;
    QLineEdit* _scriptPath;
    QPlainTextEdit* _scriptEditor;
    QPlainTextEdit* _console;
    QProgressBar* _progressBar;

    PLP::CoreI* _plpCore;
    QFuture<bool> _scriptResult;
    QTimer* _scriptRunTimer;
};

#endif // SCRIPTVIEW_H
