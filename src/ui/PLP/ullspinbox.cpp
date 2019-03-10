#include "ullspinbox.h"
#include <QLineEdit>
#include <QKeyEvent>

ULLSpinBox::ULLSpinBox(QWidget* parent) : QAbstractSpinBox (parent){
    connect(lineEdit(), SIGNAL(editingFinished()), this, SLOT(onEditFinished()));
    setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    lineEdit()->setText(QString::number(_min));
}

QSize ULLSpinBox::sizeHint() const {
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * _numDigits;
    return QSize(space, 0);
}

QAbstractSpinBox::StepEnabled ULLSpinBox::stepEnabled() const {
    return QAbstractSpinBox::StepNone;
}

void ULLSpinBox::setRange(unsigned long long min, unsigned long long max){
    if(max <= min){
        return;
    }

    _min = min;
    _max = max;

    _numDigits = 1;
    unsigned long long temp = max;
    while (temp >= 10) {
        temp /= 10;
        ++_numDigits;
    }

    lineEdit()->setText(QString::number(_min));
}

unsigned long long ULLSpinBox::value(){
    return _val;
}

void ULLSpinBox::setValue(unsigned long long val){
    if(val == _val){
        return;
    }

    if (val < _min || val > _max) {
        return;
    }
    lineEdit()->setText(QString::number(val));
    _val = val;
}

void ULLSpinBox::onEditFinished(){
    QString input = lineEdit()->text();
    unsigned long long val = input.toLongLong();
    if(val < _min){
        val = _min;
    } else if (val > _max){
        val = _max;
    }

    const unsigned long long currVal = _val;
    setValue(val);
    if(val != currVal){
        emit valueUpdated(val);
    }
}

QValidator::State ULLSpinBox::validate(QString &input, int &pos) const {
    if(input.isEmpty()){
        return QValidator::Acceptable;
    }

    bool result;
    unsigned long long val = input.toLongLong(&result);
    if (!result){
        return QValidator::Invalid;
    }
    if(pos > _numDigits){
        return QValidator::Invalid;
    }

    return QValidator::Acceptable;
}

void ULLSpinBox::fixup(QString &input) const {
    bool result;
    unsigned long long val = input.toLongLong(&result);
    if (!result){
        lineEdit()->setText(QString::number(_val));
        return;
    }

    if(val < _min){
        lineEdit()->setText(QString::number(_min));
    } else {
        lineEdit()->setText(QString::number(_max));
    }
}

void ULLSpinBox::keyPressEvent(QKeyEvent* event) {
    QAbstractSpinBox::keyPressEvent(event);
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        onEditFinished();
    }
}

