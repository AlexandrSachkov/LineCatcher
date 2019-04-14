#ifndef STANDARDSEARCHVIEW_H
#define STANDARDSEARCHVIEW_H

#include <QWidget>

#include "CoreI.h"

class StandardSearchView : public QWidget
{
    Q_OBJECT
public:
    explicit StandardSearchView(PLP::CoreI* plpCore, QWidget *parent = nullptr);

    void createSourceContent(QLayout* mainLayout);
    void createDestinationContent(QLayout* mainLayout);
    void createSearchLimiterContent(QLayout* mainLayout);
    void createSearchOptionContent(QLayout* mainLayout);
signals:

public slots:

private:
    const int ROW_LABEL_WIDTH = 300;
    PLP::CoreI* _plpCore;
    QFont fieldFont;
};

#endif // STANDARDSEARCHVIEW_H
