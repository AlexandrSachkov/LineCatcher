#include "scriptview.h"
#include <QtWidgets/QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileDialog>
#include <QLibrary>
#include <QTextStream>

ScriptView::ScriptView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    if(!_plpCore){
        //TODO
    }
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    setWindowFlags(Qt::Window);

    QHBoxLayout* scriptLoadControlLayout = new QHBoxLayout();
    mainLayout->addLayout(scriptLoadControlLayout);

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
    mainLayout->addWidget(_run);
    connect(_run, SIGNAL(clicked(void)), this, SLOT(runScript(void)));

    QSplitter* splitter = new QSplitter(this);
    mainLayout->addWidget(splitter);
    splitter->setOrientation(Qt::Orientation::Vertical);
    splitter->setHandleWidth(3);
    splitter->setChildrenCollapsible(false);

    _scriptEditor = new QPlainTextEdit(this);
    _scriptEditor->setReadOnly(false);
    splitter->addWidget(_scriptEditor);

    _console = new QPlainTextEdit(this);
    _console->setReadOnly(true);
    _console->setMaximumBlockCount(1000);
    splitter->addWidget(_console);

    _printConsole = [&](int level, const char* msg){
        _console->insertPlainText(QString::fromStdString(msg) + "\n");
    };

    plpCore->attachLogOutput("console", &_printConsole);

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
    std::wstring errMsg;
    if(!_plpCore->runScript(_scriptEditor->toPlainText().toStdWString(), errMsg)){
        _console->appendPlainText(QString::fromStdWString(errMsg));
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
    }
}
