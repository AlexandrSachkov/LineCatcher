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

#ifndef PAGEDFILEVIEW_H
#define PAGEDFILEVIEW_H

#include "ullspinbox.h"
#include "coreobjptr.h"

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPaintEvent>
#include <QSpinBox>

#include "FileReaderI.h"
#include "IndexReaderI.h"
#include <memory>
#include <vector>

class FileViewExtraSelection : public QTextEdit::ExtraSelection {
public:
    FileViewExtraSelection() : QTextEdit::ExtraSelection(){}
    unsigned long long lineNumber = 0;
};

class PagedFileViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    PagedFileViewWidget(CoreObjPtr<PLP::FileReaderI> fileReader, ULLSpinBox* lineNavBox, QWidget *parent = nullptr);
    bool getLineFromIndex(
        CoreObjPtr<PLP::IndexReaderI>& indexReader,
        QString& data
    );

    void setFontSize(int pointSize);

signals:

public slots:
    void gotoLine(unsigned long long lineNum, bool highlightLine = false);

private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void readBlockIfRequired();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    void scrollBarMoved(int val);
    void showRightClickMenu(const QPoint& pos);
    void copySelection();
    void copyLineNumber(const QPoint& pos);
    void highlightSelection();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();
    void highlightLine(unsigned long long lineNum);

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;

    CoreObjPtr<PLP::FileReaderI> _fileReader;
    unsigned int _numVisibleLines;

    QWidget* _lineNumberArea;
    ULLSpinBox* _lineNavBox;
    FileViewExtraSelection _indexSelection;
};

#endif // PAGEDFILEVIEW_H
