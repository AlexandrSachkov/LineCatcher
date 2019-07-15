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

#include "gettingstarteddialog.h"
#include "common.h"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QFile>
#include <QMessageBox>

GettingStartedDialog::GettingStartedDialog(QWidget* parent) : QWidget (parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);
    setWindowTitle("Getting Started");
    setMinimumWidth(800);
    setMinimumHeight(900);

    QString path(QString::fromStdString(Common::RESOURCE_PATH) + "/gettingstarted.html");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,"Error","Failed to open gettingstarted.html",QMessageBox::Ok);
        return;
    }

    QTextBrowser* browser = new QTextBrowser(this);
    mainLayout->addWidget(browser);
    browser->setOpenExternalLinks(true);
    browser->setHtml(file.readAll());
    browser->setSearchPaths({Common::RESOURCE_PATH});
}
