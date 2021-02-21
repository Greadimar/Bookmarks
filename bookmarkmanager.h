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
#include <QPointer>

using MBookmarkVec = QVector<MultiBookmark>;
using BookmarkVec = QVector<Bookmark>;
class BookmarkManager: public QObject
{
    Q_OBJECT
public:
    std::atomic_bool isRunning{false};
    std::atomic<int> extraTableMin{0};
    std::atomic<int> extraTableMax{0};
    int tableRowShift{0};

    BookmarkManager(QPointer<TimeAxis>& timeAxis);
    ~BookmarkManager(){
          sqlworker->closeDb();
    }
    void stop(){
        isRunning.store(false);
        bufferIsReady.store(false);
        tableBufferIsReady.store(false);
    }
    Q_INVOKABLE bool testGet();
    Q_INVOKABLE bool generateBookmarks(int count);
    Q_INVOKABLE void start(){
        isRunning = true;

        while (isRunning){
            //           qDebug() << "comp";
            QThread::sleep(0);
            collect();
            collectTableBookmarks();
        }

    }


    QFileBuffer& getFileWorker();

    // std::atomic<BookmarkVec*> readyBuffer = &bookmarksBuffer[1];
    QVector<MultiBookmark>& getToDisplay(){
        if (bufferIsReady)
            displayBuffer = readyBuffer.exchange(displayBuffer);
        auto& res = *displayBuffer;
        bufferIsReady = false;
        return res;
    }
    QVector<Bookmark>& getTableToDisplay(){
        if (tableBufferIsReady)
            displayTableBuffer = readyTableBuffer.exchange(displayTableBuffer);
        auto& res = *displayTableBuffer;
        tableBufferIsReady = false;
        return res;
    }

    SqliteWorker *getSqlworker() const;

signals:
    void sendPrg(int val);
    void serviceMsg(QString msg);
private:
    QPointer<TimeAxis> m_ta;

    MBookmarkVec bookmarksBuffer[3];
    BookmarkVec extraTableBuffer[3];
    std::atomic_bool bufferIsReady{false};
    std::atomic_bool tableBufferIsReady{false};

    std::atomic<MBookmarkVec*> displayBuffer = &bookmarksBuffer[2];
    std::atomic<MBookmarkVec*> readyBuffer = &bookmarksBuffer[1];
    std::atomic<MBookmarkVec*> computingBuffer = &bookmarksBuffer[0];

    std::atomic<BookmarkVec*> displayTableBuffer = &extraTableBuffer[2];
    std::atomic<BookmarkVec*> readyTableBuffer = &extraTableBuffer[1];
    std::atomic<BookmarkVec*> computingTableBuffer = &extraTableBuffer[0];

    std::vector<Bookmark> testList;

    QVector<QSharedPointer<MultiBookmark>> mVec;

    SqliteWorker* sqlworker;

    void collect(){
        // qDebug() << "recollect";
       // auto startCollecting = std::chrono::system_clock::now();
        int start = 0;// m_ta->getMin() - m_ta->msecFromPx(m_ta->getDragOffsetPx());
        int end = TimeInfo::msecsInDay.count();
        int duration = m_ta->getUnitingSpread();
        int d2 = m_ta->msecFromPx(m_ta->pxSpreadToUnite);
        *computingBuffer = sqlworker->getMultiBookmarks(start, end, d2);
        if (!bufferIsReady){
            computingBuffer = readyBuffer.exchange(computingBuffer);
            bufferIsReady.store(true);
        }
    }

    void collectTableBookmarks(){
        int start = extraTableMin;
        int end = extraTableMax;

        *computingTableBuffer = sqlworker->getBookmarks(start, end);
        if (!tableBufferIsReady){
            computingTableBuffer = readyTableBuffer.exchange(computingTableBuffer);
            tableBufferIsReady.store(true);
        }
    }


};

#endif // BOOKMARKMANAGER_H
