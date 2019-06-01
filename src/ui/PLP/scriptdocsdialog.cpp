#include "scriptdocsdialog.h"
#include "common.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QFile>
#include <QMessageBox>

ScriptDocsDialog::ScriptDocsDialog(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);
    setWindowTitle("Lua API Documentation");
    setMinimumWidth(800);
    setMinimumHeight(900);

    QString path(QString::fromStdString(Common::RESOURCE_PATH) + "/scriptdocs.html");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,"PLP","Failed to open scriptdocs.html",QMessageBox::Ok);
        return;
    }

    QTextBrowser* browser = new QTextBrowser(this);
    mainLayout->addWidget(browser);
    browser->setOpenExternalLinks(true);
    browser->setHtml(file.readAll());
}
