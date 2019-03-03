#include "indexviewwidget.h"
#include "signalingscrollbar.h"
#include "linenumberarea.h"
#include <QDebug>
#include <QTextBlock>
#include <QScrollBar>
#include <QPainter>


IndexViewWidget::IndexViewWidget(std::shared_ptr<PLP::FileReaderI> fileReader, QWidget* parent) : QPlainTextEdit (parent)
{
    _fileReader = fileReader;

    _lineNumberArea = new LineNumberArea(this);
    connect(_lineNumberArea, SIGNAL(paintEventOccurred(QPaintEvent* e)), this, SLOT(lineNumberAreaPaintEvent(QPaintEvent* e)));
    connect(_lineNumberArea, SIGNAL(sizeHintRequested(void)), this, SLOT(lineNumberAreaWidth(void)));

    this->setLineWrapMode(LineWrapMode::NoWrap);
    this->setWordWrapMode(QTextOption::WrapMode::NoWrap);
    this->setMaximumBlockCount(MAX_NUM_BLOCKS);

    SignalingScrollBar* scrollBar = new SignalingScrollBar();
    connect(scrollBar, SIGNAL(mouseReleased(void)), this, SLOT(readBlockIfRequired(void)));
    connect(scrollBar, SIGNAL(wheelMoved(void)), this, SLOT(readBlockIfRequired(void)));
    this->setVerticalScrollBar(scrollBar);

    QFont f("Courier New", 16);
    f.setStyleHint(QFont::Monospace);
    this->setFont(f);

    calcNumVisibleLines();
}

void IndexViewWidget::resizeEvent(QResizeEvent *e){
    QPlainTextEdit::resizeEvent(e);

    calcNumVisibleLines();
    qDebug() << "Resized" << "\n";

    QRect cr = contentsRect();
    _lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void IndexViewWidget::textChangedImpl() {
    qDebug() << "Text changed" << "\n";
    calcNumVisibleLines();
}

void IndexViewWidget::calcNumVisibleLines() {
    _numVisibleLines = this->height() / this->fontMetrics().height();
    qDebug() << "Num lines: " <<_numVisibleLines <<"\n";
}

int IndexViewWidget::lineNumberAreaWidth() const
{
    int digits = 1;
    unsigned long long max = _endLineNum > 1 ? _endLineNum : 1;
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}



void IndexViewWidget::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}



void IndexViewWidget::updateLineNumberArea(const QRect &rect, int dy)
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

void IndexViewWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(_startLineNum + blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, _lineNumberArea->width(), fontMetrics().height(), Qt::AlignLeft, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void IndexViewWidget::readNextBlock() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    for(unsigned int i = 0; i < NUM_LINES_PER_READ; i++){
        char* lineStart = nullptr;
        unsigned int length;
        if(!_fileReader->nextLine(lineStart, length)){
            break;
        }
        this->moveCursor(QTextCursor::End);
        this->insertPlainText(QString::fromUtf8(lineStart, static_cast<int>(length)));

        _endLineNum++;
        _startLineNum = _endLineNum >= MAX_NUM_BLOCKS ? _endLineNum - MAX_NUM_BLOCKS : 0;
    }

    const int newScrollbarValue = currScrollbarValue - (_startLineNum - currStartLineNum);
    this->verticalScrollBar()->setValue(newScrollbarValue);
}

void IndexViewWidget::readPreviousBlock() {
    if(_startLineNum == 0){
        return;
    }

    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;
    _startLineNum = _startLineNum >= NUM_LINES_PER_READ ? _startLineNum - NUM_LINES_PER_READ : 0;

    const unsigned long long numLinesToRead = currStartLineNum - _startLineNum;
    if(numLinesToRead == 0){
        return;
    }

    char* lineStart = nullptr;
    unsigned int length;

    //delete lines from end of document
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::End);
    for(int i = 0; i < numLinesToRead; i++){
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.deletePreviousChar();
        _endLineNum--;
    }

    //insert new lines
    cursor.movePosition(QTextCursor::Start);
    if(_fileReader->getLine(_startLineNum, lineStart, length)){
        cursor.insertText(QString::fromUtf8(lineStart, length));

        for(unsigned int i = 0; i < numLinesToRead - 1; i++){
            if(!_fileReader->nextLine(lineStart, length)){
                break;
            }
            cursor.insertText(QString::fromUtf8(lineStart, length));
        }
    }

    int newScrollbarValue = currScrollbarValue + (currStartLineNum - _startLineNum);
    this->verticalScrollBar()->setValue(newScrollbarValue);
}

void IndexViewWidget::readBlockIfRequired() {
    const int currScrollbarValue = this->verticalScrollBar()->value();
    const unsigned long long currStartLineNum = _startLineNum;

    if(_startLineNum + currScrollbarValue + _numVisibleLines > _endLineNum - (NUM_LINES_PER_READ / 2)){
        readNextBlock();
    } else if(currScrollbarValue < (NUM_LINES_PER_READ / 2)){
        readPreviousBlock();
    }
}
