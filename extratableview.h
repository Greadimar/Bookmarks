#ifndef EXTRATABLEVIEW_H
#define EXTRATABLEVIEW_H
#include <QTableView>
#include <QStyledItemDelegate>
#include "palette.h"

class Delegate;
class ExtraTableView: public QTableView
{
public:
    explicit ExtraTableView(bool& tableIsHovered,  QWidget* parent = nullptr);

private:
    bool& m_tableIsHovered;
    //bool& rowShift; //for implementing own scrollbar
    void leaveEvent(QEvent *event) override{
        m_tableIsHovered = false;
        QTableView::leaveEvent(event);
    }
    void enterEvent(QEvent *event) override{
        m_tableIsHovered = true;
        QTableView::enterEvent(event);
    }
};
class Delegate: public QStyledItemDelegate{
public:
    Delegate(const Palette& plt): m_plt(plt){

    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    const Palette& m_plt;
};


#endif // EXTRATABLEVIEW_H
