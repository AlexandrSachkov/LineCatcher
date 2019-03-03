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
