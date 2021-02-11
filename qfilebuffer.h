#ifndef QFILEBUFFER_H
#define QFILEBUFFER_H
#include <QFile>
#include <QMap>
#include <QByteArray>
#include "timeaxis.h"
#include "bookmark.h"
#include "unordered_map"
class QFileBuffer: public QObject{
    Q_OBJECT

    QFile file;
    QDataStream ds;

    QFile fileB;
    QDataStream dsB;
    QMap<msecs, QMap<msecs, int>> navMap;
signals:
    void sendPrg(int);
public:
    QFileBuffer();
    ~QFileBuffer(){file.close();}
    QByteArray buffer;
    void generateFile(QSharedPointer<TimeAxis> tc, const std::atomic_bool& isRunning, int count);
    void generateFileB(QSharedPointer<TimeAxis> tc, const std::atomic_bool& isRunning, int count);
    QVector<MultiBookmark>
    getBookmarks(msecs start, msecs end, QSharedPointer<TimeAxis> tc);
    QVector<std::variant<Bookmark, MultiBookmark>>
    getBookmarksVar(msecs start, msecs end, QSharedPointer<TimeAxis> tc);
    //       void adjustEnd(msecs spread){
    //           if (count )
    //       }
    std::optional<msecs> getLowestKey(msecs val, const QList<msecs>& keys){
        auto const it = std::lower_bound(keys.begin(), keys.end(), val);
        if (it == keys.end()) { return std::nullopt; }

        return *it;

    }

};

#endif // QFILEBUFFER_H
