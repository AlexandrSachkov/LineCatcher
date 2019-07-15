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

#ifndef ULLSPINBOX_H
#define ULLSPINBOX_H

#include <QWidget>
#include <QAbstractSpinBox>

class ULLSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
public:
    ULLSpinBox(QWidget* parent = nullptr);
    QAbstractSpinBox::StepEnabled stepEnabled() const override;

    void setRange(unsigned long long min, unsigned long long max);
    unsigned long long value();
    QSize sizeHint() const override;

signals:
    void valueUpdated(unsigned long long);

public slots:
    void setValue(unsigned long long val);
    void onEditFinished();

private:
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;
    void keyPressEvent(QKeyEvent *event) override;

    unsigned long long _min = 0;
    unsigned long long _max = 100;
    unsigned long long _val = 0;
    unsigned int _numDigits = 3;
};

#endif // ULLSPINBOX_H
