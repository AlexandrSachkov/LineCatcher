#include "scriptview.h"
#include <QtWidgets/QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QFileDialog>

ScriptView::ScriptView(QWidget *parent) : QWidget(parent)
{
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

    _run = new QPushButton("Run", this);
    mainLayout->addWidget(_run);

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
    QFile file(_scriptPath->text());
    if (file.open(QIODevice::ReadWrite)) {
        _scriptEditor->setPlainText(file.readAll());
    }
}
