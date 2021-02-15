#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H
#include "bookmark.h"
#include "timeaxis.h"
#include "sqliteworker.h"
#include <QThread>
#include <QDebug>
#include <QObject>
#include "qfilebuffer.h"
#include <atomic>
#include <QMutex>

using BookmarkVec = QVector<MultiBookmark>;
using ShpBookmarkVec = std::shared_ptr<BookmarkVec>;
class BookmarkManager: public QObject
{
    Q_OBJECT
public:
    std::atomic_bool isRunning{false};

    BookmarkManager(QSharedPointer<TimeAxis>& timeAxis);
    ~BookmarkManager(){
          sqlworker->closeDb();
    }
    void stop(){
        isRunning.store(false);
        stale.store(false);
    }
    Q_INVOKABLE bool testGet();
    Q_INVOKABLE bool generateBookmarks(int count);
    Q_INVOKABLE void start(){
        isRunning = true;

        while (isRunning){
            //           qDebug() << "comp";
            QThread::sleep(0);
            collect();
        }

    }
    std::atomic_bool toSwap{false};
    std::vector<ShpBookmark> bookmarks;

    QFileBuffer& getFileWorker();

    // std::atomic<BookmarkVec*> readyBuffer = &bookmarksBuffer[1];
    QVector<MultiBookmark>& getToDisplay(){
        if (stale)
            displayBuffer = readyBuffer.exchange(displayBuffer);
        auto& res = *displayBuffer;
        stale = false;
        return res;
    }

    std::atomic_bool displayBufIsBusy{false};
    std::atomic_bool stale{false};
    QMutex mutex;
signals:
    void sendPrg(int val);
    void serviceMsg(QString msg);
private:
    QSharedPointer<TimeAxis> m_ta;
    BookmarkVec bookmarksBuffer[3];
    std::atomic<BookmarkVec*> displayBuffer = &bookmarksBuffer[2];
    std::atomic<BookmarkVec*> readyBuffer = &bookmarksBuffer[1];
    std::atomic<BookmarkVec*> computingBuffer = &bookmarksBuffer[0];
    std::vector<Bookmark> testList;

    QVector<QSharedPointer<MultiBookmark>> mVec;

    SqliteWorker* sqlworker;
    //   QFileBuffer m_fileworker;

    //  int curRulerWidth{0};
    void collect(){
        // qDebug() << "recollect";
        auto startCollecting = std::chrono::system_clock::now();
        int start = m_ta->getMin();
        int end = m_ta->getMax();
        int duration = m_ta->getUnitingSpread();

        *computingBuffer = sqlworker->getBookmarks(start, end, duration);
        //while (stale.exchange(true));
        if (!stale){
            computingBuffer = readyBuffer.exchange(computingBuffer);
            stale.store(true);
        }


        auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
        //    qDebug() << "timeToReCollectVec" << timeToCollect.count();
    }


};

#endif // BOOKMARKMANAGER_H
