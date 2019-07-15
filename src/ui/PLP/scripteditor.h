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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "luasyntaxhighlighter.h"
#include <QPlainTextEdit>

class ScriptEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    ScriptEditor(QWidget* parent = nullptr);
    void setFontSize(int pointSize);
protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;

private:
    QWidget* _lineNumberArea;
    LuaSyntaxHighlighter* _highlighter;
};

#endif // SCRIPTEDITOR_H
