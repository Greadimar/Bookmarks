#include "extratablemodel.h"
#include <QApplication>
#include <QFontMetrics>
int ExtraTableModel::rowCount(const QModelIndex &) const
{
    return curVec.size();
}

int ExtraTableModel::columnCount(const QModelIndex &) const
{
    return headers.count();
}

void ExtraTableModel::updateModel()
{
    curVec = m_bmkMngr->getTableToDisplay();
    emit dataChanged(QModelIndex(), QModelIndex());
    emit layoutChanged();
}
QVariant ExtraTableModel::data(const QModelIndex &index, int role) const
{
    // qDebug() << "DATA" << index.row();
    if (index.isValid() && role == Qt::DisplayRole){
        if (index.row() > curVec.size()) return QVariant();
        if (index.row() == displayLimit){
            return QString("...");
        }
        const auto& bk = curVec[index.row()];
        if (index.column() == NumHeaders::name) return bk.name;
        if (index.column() == NumHeaders::start) return timeFromMsec(bk.start);
        if (index.column() == NumHeaders::end) return timeFromMsec(bk.end);
    }

    return QVariant();
}
QVariant ExtraTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (role == Qt::SizeHintRole && orientation == Qt::Orientation::Vertical) return 1;
    if (role == Qt::SizeHintRole && orientation == Qt::Orientation::Horizontal){
        QFontMetrics fm = qApp->fontMetrics();
            int paddings = fm.ascent()*2;
        switch (section) {
        case NumHeaders::name:
            return fm.horizontalAdvance("bookmark 999999999") + paddings;
        default:
            return fm.horizontalAdvance("hh:mm:ss:zzzz") + paddings;

        }
    }
    if (orientation == Qt::Horizontal) {
        return headers.at(section);
    }
    return QVariant();
}
