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

#ifndef INDEXVIEW_H
#define INDEXVIEW_H

#include <QWidget>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
#include "indexviewwidget.h"
#include "ullspinbox.h"
#include "IndexReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexView : public QWidget
{
    Q_OBJECT
public:
    explicit IndexView(
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        QWidget *parent = nullptr
    );

    const QString& getFilePath();
    void setFontSize(int pointSize);

private:
    QString _filePath;
    IndexViewWidget* _indexView;
};

#endif // INDEXVIEW_H
