#include "highlightsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QRandomGenerator>
#include <QColorDialog>

HighlightItem::HighlightItem(
        const QString& pattern,
        bool regex,
        const QColor& color,
        std::function<void()> highlightUpdated,
        QWidget* parent
) : QWidget(parent){
    _highlightUpdated = highlightUpdated;

    QHBoxLayout* mainLayout = new QHBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(5, 0, 0, 0);
    mainLayout->setSpacing(5);
    setToolTip(pattern);

    _pattern = new QLabel(pattern, this);
    _pattern->setMaximumWidth(400);
    mainLayout->addWidget(_pattern);

    _regex = regex;

    _colorPicker = new QPushButton(this);
    mainLayout->addWidget(_colorPicker, 1, Qt::AlignRight);
    connect(_colorPicker, &QPushButton::clicked, this, &HighlightItem::changeColor);
    updateColor(color);

    _remove = new QPushButton(this);
    _remove->setIcon(this->style()->standardIcon(QStyle::SP_DialogCloseButton));

    connect(_remove, &QPushButton::clicked, [&, parent](){
        if(parent){
            static_cast<HighlightsDialog*>(parent)->removeItem(_pattern->text(), _regex);
        }
    });
    mainLayout->addWidget(_remove);
}

void HighlightItem::changeColor() {
    QColor newColor = QColorDialog::getColor(_color);
    if(newColor.isValid()){
         updateColor(newColor);
         _highlightUpdated();
    }
}

void HighlightItem::updateColor(const QColor& color) {
    _color = color;
    QString strColor = "rgb(" + QString::number(color.red()) + "," +
            QString::number(color.green()) + "," +
            QString::number(color.blue()) + ")";

    _colorPicker->setStyleSheet(
        "QPushButton {"
            "background-color: "+ strColor +"; border: 1px solid black;"
            "min-width: 50px;"
            "height: 18px;"
            "padding: 0px;"
            "margin: 0px;"
       "}"
    );
}

QString HighlightItem::getPattern() const {
    return _pattern->text();
}

bool HighlightItem::getRegex() const {
    return _regex;
}

QColor HighlightItem::getColor() const {
    return _color;
}

HighlightsDialog::HighlightsDialog(std::function<void()> highlightsUpdated, QWidget *parent) : QWidget(parent)
{
    _highlightsUpdated = highlightsUpdated;
    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle("Highlights");
    setFixedSize(500, 600);

    QHBoxLayout* menuLayout = new QHBoxLayout();
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);
    mainLayout->addLayout(menuLayout);

    QHBoxLayout* patternLayout = new QHBoxLayout();
    patternLayout->setContentsMargins(5, 0, 0, 0);
    patternLayout->setSpacing(5);
    menuLayout->addLayout(patternLayout);

    QLabel* patternLabel = new QLabel("Pattern:");
    patternLayout->addWidget(patternLabel);

    _pattern = new QLineEdit(this);
    patternLayout->addWidget(_pattern);

    _regex = new QCheckBox("Regex", this);
    patternLayout->addWidget(_regex);

    _addItem = new QPushButton("Add", this);
    menuLayout->addWidget(_addItem, 1, Qt::AlignRight);
    connect(_addItem, &QPushButton::clicked, [this](){
        addHighlight(_pattern->text(), _regex->isChecked());
    });

    _clear = new QPushButton("Clear", this);
    menuLayout->addWidget(_clear);
    connect(_clear, &QPushButton::clicked, this, &HighlightsDialog::clear);

    _highlightItems = new QListWidget(this);
    mainLayout->addWidget(_highlightItems);

    QFont font = this->font();
    font.setPointSize(12);
    this->setFont(font);
}

void HighlightsDialog::addHighlight(const QString& pattern, bool regex) {
    int r = QRandomGenerator::global()->bounded(100, 256);
    int g = QRandomGenerator::global()->bounded(100, 256);
    int b = QRandomGenerator::global()->bounded(100, 256);

    addHighlight(pattern, regex, QColor(r,g,b));
}

void HighlightsDialog::addHighlight(const QString& pattern, bool regex, const QColor& color) {
    if(pattern.isEmpty()){
        return;
    }

    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(item->sizeHint().width(), 23));
    _highlightItems->addItem(item);
    _highlightItems->setItemWidget(item, new HighlightItem(pattern, regex, color, _highlightsUpdated, this));

    _highlightsUpdated();
}

void HighlightsDialog::removeItem(const QString& pattern, bool regex) {
    for(int i = 0; i < _highlightItems->count(); i++){
        HighlightItem* item = static_cast<HighlightItem*>(_highlightItems->itemWidget(_highlightItems->item(i)));
        if(item->getPattern() == pattern && item->getRegex() == regex){
            _highlightItems->takeItem(i);
            break;
        }
    }

    _highlightsUpdated();
}

QList<const HighlightItem*> HighlightsDialog::getHighlights() {
    QList<const HighlightItem*> highlights;
    for(int i = 0; i < _highlightItems->count(); i++){
        highlights.push_back(static_cast<HighlightItem*>(_highlightItems->itemWidget(_highlightItems->item(i))));
    }

    return highlights;
}

void HighlightsDialog::clear() {
    _highlightItems->clear();
    _highlightsUpdated();
}
