#ifndef SIGNALINGSCROLLBAR_H
#define SIGNALINGSCROLLBAR_H

#include <QScrollBar>

class SignalingScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit SignalingScrollBar(QWidget* parent = nullptr) : QScrollBar(parent) {}

    void mouseReleaseEvent(QMouseEvent* e) override {
        QScrollBar::mouseReleaseEvent(e);
        emit mouseReleased();
    }

    void wheelEvent(QWheelEvent* e) override {
        QScrollBar::wheelEvent(e);
        emit wheelMoved();
    }

signals:
    void mouseReleased();
    void wheelMoved();
};

#endif // SIGNALINGSCROLLBAR_H
