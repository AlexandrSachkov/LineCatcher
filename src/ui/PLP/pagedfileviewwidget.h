#ifndef PAGEDFILEVIEW_H
#define PAGEDFILEVIEW_H

#include <QPlainTextEdit>
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

protected:
    void resizeEvent(QResizeEvent *e) override;
    void scrollContentsBy(int dx, int dy) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    void calcNumVisibleLines();

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

#endif // PAGEDFILEVIEW_H
