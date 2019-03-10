#ifndef ULLSPINBOX_H
#define ULLSPINBOX_H

#include <QWidget>
#include <QAbstractSpinBox>

class ULLSpinBox : public QAbstractSpinBox
{
    Q_OBJECT
public:
    ULLSpinBox(QWidget* parent = nullptr);
    void stepBy(int step) override;
    QAbstractSpinBox::StepEnabled stepEnabled() const override;

    void setRange(unsigned long long min, unsigned long long max);
    unsigned long long value();
    QSize sizeHint() const override;

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
