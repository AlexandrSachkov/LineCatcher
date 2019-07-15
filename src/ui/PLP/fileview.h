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

#ifndef FILEVIEW_H
#define FILEVIEW_H

#include "pagedfileviewwidget.h"
#include "ullspinbox.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QSplitter>

#include "coreobjptr.h"
#include "FileReaderI.h"
#include "IndexReaderI.h"

#include <memory>

class FileView : public QWidget
{
    Q_OBJECT
public:
    explicit FileView(CoreObjPtr<PLP::FileReaderI> fileReader, QWidget *parent = nullptr);
    ~FileView();

    const QString& getFilePath();
    void openIndex(CoreObjPtr<PLP::IndexReaderI> indexReader);
    void setFontSize(int pointSize);
signals:

private slots:
    void closeTab(int index);

private:
    PagedFileViewWidget* _dataView;
    QTabWidget* _indexViewer;
    ULLSpinBox* _currLineNumBox;
    QSplitter* _splitter;

    QString _filePath;
};

#endif // FILEVIEW_H
