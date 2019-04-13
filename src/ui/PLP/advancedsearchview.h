#ifndef ADVANCEDSEARCHVIEW_H
#define ADVANCEDSEARCHVIEW_H

#include <QWidget>

#include "CoreI.h"

class AdvancedSearchView : public QWidget
{
    Q_OBJECT
public:
    explicit AdvancedSearchView(PLP::CoreI* plpCore, QWidget *parent = nullptr);

signals:

public slots:

private:
    PLP::CoreI* _plpCore;
};

#endif // ADVANCEDSEARCHVIEW_H
