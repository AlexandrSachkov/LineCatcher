#include "advancedsearchview.h"

#include <QBoxLayout>

AdvancedSearchView::AdvancedSearchView(PLP::CoreI* plpCore, QWidget *parent) : QWidget(parent)
{
    _plpCore = plpCore;

    QVBoxLayout* mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);

    setWindowFlags(Qt::Window);
    setWindowTitle("Advanced Search");
}
