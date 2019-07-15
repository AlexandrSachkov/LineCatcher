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

#include "scripteditor.h"
#include "linenumberarea.h"
#include "common.h"

#include <QTextBlock>
#include <QPainter>
#include <QFontMetricsF>

ScriptEditor::ScriptEditor(QWidget* parent) : QPlainTextEdit (parent)
{
    _lineNumberArea = new LineNumberArea(this);
    connect(_lineNumberArea, SIGNAL(paintEventOccurred(QPaintEvent*)), this, SLOT(lineNumberAreaPaintEvent(QPaintEvent*)));
    connect(_lineNumberArea, SIGNAL(sizeHintRequested(void)), this, SLOT(lineNumberAreaWidth(void)));

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

    this->setLineWrapMode(LineWrapMode::NoWrap);
    this->setWordWrapMode(QTextOption::WrapMode::NoWrap);

    _highlighter = new LuaSyntaxHighlighter(this->document());

    QFont f("Courier New", 14);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);

    const int tabStop = 4;  // 4 characters
    QFontMetricsF metrics(this->font());
    this->setTabStopDistance(metrics.width(' ') * tabStop);
}

void ScriptEditor::resizeEvent(QResizeEvent *e){
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    _lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

int ScriptEditor::lineNumberAreaWidth() const
{
    unsigned long long max = qMax(1, blockCount());

    int digits = 1;
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}



void ScriptEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy){
        _lineNumberArea->scroll(0, dy);
    } else {
        _lineNumberArea->update(0, rect.y(), _lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(_lineNumberArea);
    painter.fillRect(event->rect(), Common::LineNumberAreaBGColor);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Common::LineHighlightTextColor);
            painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignLeft, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ScriptEditor::setFontSize(int pointSize) {
    QFont font = this->font();
    font.setPointSize(pointSize);
    this->setFont(font);
}
