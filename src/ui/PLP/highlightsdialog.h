#ifndef HIGHLIGHTSDIALOG_H
#define HIGHLIGHTSDIALOG_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

class HighlightItem : public QWidget
{
    Q_OBJECT
public:
    explicit HighlightItem(const QString& pattern, bool regex, const QColor& color, std::function<void()> highlightUpdated, QWidget* parent = nullptr);
    QString getPattern() const;
    bool getRegex() const;
    QColor getColor() const;
private:
    void changeColor();
    void updateColor(const QColor& color);

    std::function<void()> _highlightUpdated;
    QLabel* _pattern = nullptr;
    QPushButton* _colorPicker = nullptr;
    QPushButton* _remove = nullptr;
    bool _regex = false;
    QColor _color;
};

class HighlightsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit HighlightsDialog(std::function<void()> highlightsUpdated, QWidget* parent = nullptr);
    void addHighlight(const QString& pattern, bool regex);
    void addHighlight(const QString& pattern, bool regex, const QColor& color);
    void removeItem(const QString& pattern, bool regex);
    void clear();
    QList<const HighlightItem*> getHighlights();
private:
    std::function<void()> _highlightsUpdated;
    QListWidget* _highlightItems = nullptr;
    QLineEdit* _pattern = nullptr;
    QCheckBox* _regex = nullptr;
    QPushButton* _addItem = nullptr;
    QPushButton* _clear = nullptr;
};

#endif // HIGHLIGHTSDIALOG_H
