#ifndef EXTRATABLEMODEL_H
#define EXTRATABLEMODEL_H
#include "timeaxis.h"
#include "bookmarkmanager.h"
#include <QPointer>
#include <QAbstractTableModel>
#include <QTimer>
#include <ctime>
const int displayLimit{16};

class ExtraTableModel: public QAbstractTableModel
{
public:
    explicit ExtraTableModel(QPointer<TimeAxis>& timeAxis,
                    QPointer<BookmarkManager> bmkMngr, const msecs& renderStep, QObject* parent) : QAbstractTableModel(parent),
        m_ta(timeAxis), m_bmkMngr(bmkMngr), m_renderStep(renderStep) {
        timer = new QTimer(parent);
        connect(timer, &QTimer::timeout, this, &ExtraTableModel::updateModel);
        timer->start(renderStep);
        curVec << Bookmark(9999,9999);
        curVec.last().setName("bookmark 99999999");
    }
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
    QPointer<TimeAxis> m_ta;
    QPointer<BookmarkManager> m_bmkMngr;
    const msecs& m_renderStep;
    QVector<Bookmark> curVec;
    enum NumHeaders : int{
        name = 0, start = 1, end = 2
    };
    QStringList headers{
        "имя",
        "начало",
        "конец"
    };

    int rowCount(const QModelIndex&) const override;
    int columnCount(const QModelIndex&) const override;
    QTimer* timer;

    void updateModel();
    static QString timeFromMsec(int msec){
        QTime time(0,0);
        time = time.addMSecs(msec);
        return time.toString("hh:mm:ss:zzz");
    }
};


#endif // EXTRATABLEMODEL_H
