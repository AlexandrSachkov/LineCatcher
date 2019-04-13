#ifndef STANDARDSEARCHVIEW_H
#define STANDARDSEARCHVIEW_H

#include <QWidget>

#include "CoreI.h"

class StandardSearchView : public QWidget
{
    Q_OBJECT
public:
    explicit StandardSearchView(PLP::CoreI* plpCore, QWidget *parent = nullptr);

signals:

public slots:

private:
    PLP::CoreI* _plpCore;
};

#endif // STANDARDSEARCHVIEW_H
