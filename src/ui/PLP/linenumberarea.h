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

#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>
#include <QPaintEvent>

class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(QWidget* parent = nullptr) : QWidget(parent) {}

    QSize sizeHint() const override {
        return QSize(emit sizeHintRequested(), 0);
    }

protected:
    void paintEvent(QPaintEvent* e) override {
        emit paintEventOccurred(e);
    }

signals:
    void paintEventOccurred(QPaintEvent* e);
    int sizeHintRequested() const;
};

#endif // LINENUMBERAREA_H
