#ifndef PAGEDFILEVIEW_H
#define PAGEDFILEVIEW_H

#include <QPlainTextEdit>
#include <QScrollBar>

#include "FileReaderI.h"
#include <memory>

class PagedFileViewWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    PagedFileViewWidget(std::unique_ptr<PLP::FileReaderI> fileReader, QWidget *parent = nullptr);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

signals:

private slots:
    void textChangedImpl();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect&, int);
    void readBlockIfRequired();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void calcNumVisibleLines();
    void readNextBlock();
    void readPreviousBlock();

    static const unsigned int MAX_NUM_BLOCKS = 1000;
    static const unsigned int NUM_LINES_PER_READ = 250;

    unsigned long long _startLineNum = 0;
    unsigned long long _endLineNum = 0;

    std::unique_ptr<PLP::FileReaderI> _fileReader;
    unsigned int _numVisibleLines;

    QWidget* _lineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(PagedFileViewWidget* fileViewWidget) : QWidget(fileViewWidget) {
        _fileViewWidget = fileViewWidget;
    }

    QSize sizeHint() const override {
        return QSize(_fileViewWidget->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        _fileViewWidget->lineNumberAreaPaintEvent(event);
    }

private:
    PagedFileViewWidget* _fileViewWidget;
};

class SignalingScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    SignalingScrollBar(QWidget* parent = nullptr) : QScrollBar(parent) {}

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

#endif // PAGEDFILEVIEW_H
