#include "scriptview.h"
#include <QtWidgets/QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileDialog>
#include <QLibrary>
#include <QTextStream>
#include <QScrollBar>

const char* ScriptView::LOG_SUBSCRIBER_NAME = "console";

ScriptView::ScriptView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    if(!_plpCore){
        //TODO
    }
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);

    QHBoxLayout* scriptLoadControlLayout = new QHBoxLayout();
    mainLayout->addLayout(scriptLoadControlLayout);
    scriptLoadControlLayout->setContentsMargins(0, 0, 0, 0);


    _scriptPath = new QLineEdit(this);
    scriptLoadControlLayout->addWidget(_scriptPath);

    _open = new QPushButton("Open", this);
    scriptLoadControlLayout->addWidget(_open);
    connect(_open, SIGNAL(clicked(void)), this, SLOT(openScript(void)));

    _load = new QPushButton("Load", this);
    scriptLoadControlLayout->addWidget(_load);
    connect(_load, SIGNAL(clicked(void)), this, SLOT(loadScript(void)));

    _save = new QPushButton("Save", this);
    scriptLoadControlLayout->addWidget(_save);
    connect(_save, SIGNAL(clicked(void)), this, SLOT(saveScript(void)));

    _run = new QPushButton("Run", this);
    scriptLoadControlLayout->addWidget(_run);
    connect(_run, SIGNAL(clicked(void)), this, SLOT(runScript(void)));


    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(0);
    splitter->setChildrenCollapsible(false);

    _scriptEditor = new QPlainTextEdit(this);
    _scriptEditor->setReadOnly(false);

    QWidget* consoleWidget = new QWidget(splitter);
    QVBoxLayout* consoleLayout = new QVBoxLayout();
    consoleWidget->setLayout(consoleLayout);
    consoleLayout->setContentsMargins(0, 0, 0, 0);
    consoleLayout->setSpacing(0);

    _clearConsole = new QPushButton("Clear", consoleWidget);
    _clearConsole->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    consoleLayout->addWidget(_clearConsole);
    connect(_clearConsole, SIGNAL(clicked(void)), this, SLOT(clearConsole(void)));

    _console = new QPlainTextEdit(consoleWidget);
    _console->setReadOnly(true);
    _console->setMaximumBlockCount(1000);
    consoleLayout->addWidget(_console);

    splitter->addWidget(_scriptEditor);
    splitter->addWidget(consoleWidget);

    _printConsole = [&](int level, const char* msg){
        QString levelName;
        QColor textColor;

        if(level == 1){
            levelName = "Warning: ";
            textColor = QColor(255,150,0);
        } else if(level >= 2){
            levelName = "Error: ";
            textColor = QColor(255,0,0);
        }else{
            levelName = ""; //Info is not displayed
            textColor = QColor(0,0,0);
        }

        QTextCharFormat tf = _console->currentCharFormat();
        tf = _console->currentCharFormat();
        tf.setForeground(QBrush(textColor));

        QTextCursor cursor = _console->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.setCharFormat(tf);
        cursor.insertText(levelName + QString::fromStdString(msg) + "\n");

        QScrollBar* scrollBar = _console->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    };

    QFont f("Courier New", 14);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);
}

ScriptView::~ScriptView() {

}

void ScriptView::openScript(){
    QString path = QFileDialog::getOpenFileName(this, "Select file to open");
    if(path.isEmpty()){
        return;
    }

    QString fileName = path.split('/').last();
    _scriptPath->setText(path);

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _scriptEditor->setPlainText(file.readAll());
    }
}

void ScriptView::loadScript() {
    QString path = _scriptPath->text().trimmed();
    if(path.isEmpty()){
        return;
    }

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _scriptEditor->setPlainText(file.readAll());
    }
}

void ScriptView::runScript() {
    _plpCore->attachLogOutput(LOG_SUBSCRIBER_NAME, &_printConsole);

    std::wstring errMsg;
    if(!_plpCore->runScript(_scriptEditor->toPlainText().toStdWString(), errMsg)){
        _console->appendPlainText(QString::fromStdWString(errMsg));
    }

    _plpCore->detachLogOutput(LOG_SUBSCRIBER_NAME);
}

void ScriptView::saveScript() {
    QString path = _scriptPath->text().trimmed();
    if(path.isEmpty()){
        return;
    }

    QString script = _scriptEditor->toPlainText().trimmed();

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << script;
    }
}

void ScriptView::clearConsole() {
    _console->clear();
}
