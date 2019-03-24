#include "scriptview.h"
#include "scripteditor.h"

#include <QtWidgets/QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileDialog>
#include <QLibrary>
#include <QTextStream>
#include <QScrollBar>
#include <QSettings>
#include <QApplication>
#include <QDesktopWidget>

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

    _scriptEditor = new ScriptEditor(this);
    _scriptEditor->setReadOnly(false);
    connect(_scriptEditor, SIGNAL(textChanged(void)), this, SLOT(onScriptModified(void)));

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

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    splitter->setSizes(QList<int>({(int)(screenGeometry.height() / 4 * 2.5), (int)(screenGeometry.height() / 4 * 1.5)}));

    _progressBar = new QProgressBar(this);
    _progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _progressBar->setTextVisible(false);
    _progressBar->setRange(0,0);
    _progressBar->setMaximumHeight(10);
    _progressBar->setHidden(true);
    mainLayout->addWidget(_progressBar);

    _scriptRunTimer = new QTimer(this);
    connect(_scriptRunTimer, SIGNAL(timeout()), this, SLOT(checkScriptCompleted()));

    _appendLogData = [&](int level, const char* msg){
        std::lock_guard<std::mutex> guard(_logDataLock);
        _logData.push_back({level, QString::fromStdString(msg)});
    };

    QFont f("Courier New", 14);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);

    setScriptModified(false);
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

    loadScript();
}

void ScriptView::loadScript() {
    QString path = _scriptPath->text().trimmed();
    if(path.isEmpty()){
        return;
    }

    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _scriptEditor->setPlainText(file.readAll());
        setScriptModified(false);
    }
}

void ScriptView::runScript() {
    if(_run->text().compare("Run") == 0){ //TODO refactor to keep state in button
        _plpCore->attachLogOutput(LOG_SUBSCRIBER_NAME, &_appendLogData);
        std::wstring script = _scriptEditor->toPlainText().toStdWString();

        _scriptResult = QtConcurrent::run([&, script](){
            return _plpCore->runScript(&script);
        });
        _run->setText("Stop"); //TODO refactor to keep state in button
        _progressBar->setHidden(false);
        _appendLogData(0, "======== Script started ========");
        _scriptRunTimer->start(250);
    }else{
        _plpCore->cancelOperation();
    }
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

        setScriptModified(false);
    }
}

void ScriptView::clearConsole() {
    _console->clear();
}

void ScriptView::hideEvent(QHideEvent* event) {
    QSettings settings("AlexandrSachkov", "PLP");
    settings.beginGroup("ScriptView");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();
}

void ScriptView::showEvent(QShowEvent* event) {
    QSize defaultSize = parentWidget()->size() * 0.75;

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int defaultPosX = (screenGeometry.width() - defaultSize.width()) / 2;
    int defaultPosY = (screenGeometry.height() - defaultSize.height()) / 2;

    QSettings settings("AlexandrSachkov", "PLP");
    settings.beginGroup("ScriptView");
    resize(settings.value("size", defaultSize).toSize());
    move(settings.value("pos", QPoint(defaultPosX, defaultPosY)).toPoint());

    bool maximized = settings.value("maximized", false).toBool();
    if(maximized){
        showMaximized();
    }

    settings.endGroup();
}

void ScriptView::onScriptModified(){
    setScriptModified(true);
}

void ScriptView::setScriptModified(bool modified) {
    _saved = !modified;

    QPalette p = _save->palette();
    if(modified){
        p.setColor(QPalette::Button, QColor(255,0,0));
        p.setColor(QPalette::ButtonText, QColor(255,0,0));
    }else{
        p.setColor(QPalette::Button, QColor(0,255,0));
        p.setColor(QPalette::ButtonText, QColor(0,0,0));
    }
    _save->setAutoFillBackground(true);
    _save->setPalette(p);
    _save->update();
}

void ScriptView::checkScriptCompleted() {
    if(_scriptResult.isFinished()){
        _scriptRunTimer->stop();
        _plpCore->detachLogOutput(LOG_SUBSCRIBER_NAME);
        _appendLogData(0, "======== Script finished ========");
        _progressBar->setHidden(true);
        _run->setText("Run"); //TODO refactor to keep state in button
    }
    printLogDataToConsole();
}

void ScriptView::printLogDataToConsole() {
    std::lock_guard<std::mutex> guard(_logDataLock);
    for(auto& pair : _logData){
        int level = pair.first;
        QString& msg = pair.second;

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
        cursor.insertText(levelName + msg + "\n");
    }

    if(_logData.size() > 0){
        QScrollBar* scrollBar = _console->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    _logData.clear();
}
