#include "extratableview.h"

ExtraTableView::ExtraTableView(bool& tableIsHovered, QWidget *parent) : QTableView(parent),
    m_tableIsHovered(tableIsHovered)
{
    //set
}
void Delegate::paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
{
    QStyleOptionViewItem op(option);

//    if (index.row() == 2) {
//        op.font.setBold(true);
//        op.palette.setColor(QPalette::Normal, QPalette::Background, Qt::black);
//        op.palette.setColor(QPalette::Normal, QPalette::Foreground, Qt::white);
//    }
    QStyledItemDelegate::paint(painter, op, index);
}

