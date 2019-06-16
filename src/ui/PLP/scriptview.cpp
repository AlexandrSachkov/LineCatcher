#include "scriptview.h"
#include "scripteditor.h"
#include "common.h"

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
#include <QMessageBox>

const char* ScriptView::LOG_SUBSCRIBER_NAME = "console";

ScriptView::ScriptView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);
    setWindowTitle("Script Editor");

    QHBoxLayout* scriptLoadControlLayout = new QHBoxLayout();
    mainLayout->addLayout(scriptLoadControlLayout);
    scriptLoadControlLayout->setContentsMargins(0, 0, 0, 0);


    _scriptPath = new QLineEdit(this);
    _scriptPath->setReadOnly(true);
    scriptLoadControlLayout->addWidget(_scriptPath);

    _open = new QPushButton("Browse", this);
    scriptLoadControlLayout->addWidget(_open);
    connect(_open, SIGNAL(clicked(void)), this, SLOT(openScript(void)));

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
    _console->setMaximumBlockCount(MAX_LINES_CONSOLE);
    consoleLayout->addWidget(_console);

    QFont consoleFont("Courier New", 12);
    consoleFont.setStyleHint(QFont::Monospace);
    _console->setFont(consoleFont);

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

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);

    // PlainTextEdit emits textChanged signal when first initialized. Therefore, we connect after its initialization
    QTimer::singleShot(0, this, [&](){
        connect(_scriptEditor, &ScriptEditor::textChanged, [&](){
            setScriptModified(true);
        });
        setScriptModified(false);
    });
}

ScriptView::~ScriptView() {
    _plpCore->detachLogOutput(LOG_SUBSCRIBER_NAME);
}

void ScriptView::openScript(){
    QFileDialog fileDialog(this, "Select file to open", "", tr("Lua (*.lua)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);

    if (!fileDialog.exec()){
        return;
    }

    QStringList paths = fileDialog.selectedFiles();
    QString path = paths[0].trimmed();
    if(path.isEmpty()){
        return;
    }

    QString scriptDir = Common::getDirFromPath(path);
    QString fileName = path.split('/').last();
    if(!fileName.endsWith(".lua")){
        fileName += ".lua";
    }

    path = scriptDir + "/" + fileName;
    _scriptPath->setText(path);

    _file.reset(new QFile(path));
    if (_file->open(QIODevice::ReadWrite | QIODevice::Text)) {
        _scriptEditor->setPlainText(_file->readAll());
        setScriptModified(false);
    }else{
        QMessageBox::critical(this,"Error","Failed to open script: " + path, QMessageBox::Ok);
    }
}

void ScriptView::runScript() {
    if(_run->text().compare("Run") == 0){ //TODO refactor to keep state in button
        _plpCore->attachLogOutput(LOG_SUBSCRIBER_NAME, &_appendLogData);
        std::wstring script = _scriptEditor->toPlainText().toStdWString();

        _appendLogData(0, "======== Script started  ========");
        _scriptResult = QtConcurrent::run([&, script](){
            return _plpCore->runScript(&script);
        });
        _run->setText("Stop"); //TODO refactor to keep state in button
        _progressBar->setHidden(false); 
        _scriptRunTimer->start(250);
    }else{
        _plpCore->cancelOperation();
    }
}

bool ScriptView::saveScript() {
    // path is only unavailable when no script was selected. Therefore, nothing to save
    QString path = _scriptPath->text().trimmed();
    if(path.isEmpty()){
        return true;
    }

    QString script = _scriptEditor->toPlainText().trimmed();
    if(_file){
        QTextStream out(_file.get());
        out << script;

        if (out.status() != QTextStream::Ok)
        {
            QMessageBox::critical(this,"Error","Failed to save script", QMessageBox::Ok);
            return false;
        }

        setScriptModified(false);
    }
    return true;
}

void ScriptView::clearConsole() {
    _console->clear();
}

void ScriptView::hideEvent(QHideEvent* event) {
    QSettings settings("AlexandrSachkov", "LC");
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

    QSettings settings("AlexandrSachkov", "LC");
    settings.beginGroup("ScriptView");
    resize(settings.value("size", defaultSize).toSize());
    move(settings.value("pos", QPoint(defaultPosX, defaultPosY)).toPoint());

    bool maximized = settings.value("maximized", false).toBool();
    if(maximized){
        showMaximized();
    }

    settings.endGroup();
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

    QTextCharFormat tf = _console->currentCharFormat();
    QTextCursor cursor = _console->textCursor();

    //print at most number of lines that fit into console and delete the rest
    const int start = _logData.size() > MAX_LINES_CONSOLE ? _logData.size() - MAX_LINES_CONSOLE : 0;
    for(int i = start; i < _logData.size(); i++) {
        auto& pair = _logData[i];
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

        tf.setForeground(QBrush(textColor));
        cursor.setCharFormat(tf);
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(levelName + msg + "\n");
    }

    if(_logData.size() > 0){
        QScrollBar* scrollBar = _console->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
    _logData.clear();
}

void ScriptView::setFontSize(int pointSize) {
    _scriptEditor->setFontSize(pointSize);

    QFont font = _console->font();
    font.setPointSize(pointSize);
    _console->setFont(font);
}

bool ScriptView::hasUnsavedContent(){
    return !_saved;
}
