#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QLineEdit>


class ScriptView : public QWidget
{
    Q_OBJECT
public:
    explicit ScriptView(QWidget *parent = nullptr);
    ~ScriptView();
signals:

public slots:
    void openScript();
    void loadScript();

private:
    QPushButton* _open;
    QPushButton* _load;
    QPushButton* _run;
    QLineEdit* _scriptPath;
    QPlainTextEdit* _scriptEditor;
    QPlainTextEdit* _console;
};

#endif // SCRIPTVIEW_H
