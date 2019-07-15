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

#ifndef INDEXVIEWWIDGET_H
#define INDEXVIEWWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPaintEvent>
#include <QMouseEvent>

#include "coreobjptr.h"
#include "pagedfileviewwidget.h"
#include "ullspinbox.h"
#include "IndexReaderI.h"
#include "FileReaderI.h"
#include <memory>

class IndexViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    IndexViewWidget(
        CoreObjPtr<PLP::IndexReaderI> indexReader,
        PagedFileViewWidget* fileViewer,
        ULLSpinBox* lineNavBox,
        QWidget* parent = nullptr
    );

    void setFontSize(int pointSize);

protected:
    void resizeEvent(QResizeEvent *e) override;
public slots:
    void gotoLine(unsigned long long lineNum);
private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    void readBlockIfRequired();
    void scrollBarMoved(int val);

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();
    void mouseReleaseEvent(QMouseEvent* event) override;

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    CoreObjPtr<PLP::IndexReaderI> _indexReader;
    PagedFileViewWidget* _fileViewer;
    QWidget* _lineNumberArea;
    ULLSpinBox* _lineNavBox;
    std::vector<unsigned long long> _indices;
    std::vector<QString> _data;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;
    unsigned int _numVisibleLines = 0;
};

#endif // INDEXVIEWWIDGET_H
