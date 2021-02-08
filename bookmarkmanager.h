#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H
#include "bookmark.h"
#include "timeconvertor.h"
//#include "sqliteworker.h"
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

    BookmarkManager(QSharedPointer<TimeConvertor>& convertor): QObject(nullptr),

//        bookmarksToDisplay(&bookmarksBuffer1),
//        bookmarksToCompute(&bookmarksBuffer2),
        m_timeconvertor(convertor)
    {
        //sqlworker = new SqliteWorker(isRunning);
    };
    void stop(){
        isRunning.store(false);
    }
    Q_INVOKABLE bool generateBookmarks(int count);
    Q_INVOKABLE void start(){
        isRunning = true;
        while (isRunning){
            QThread::sleep(0);
           // if (curRulerWidth == m_timeconvertor.rulerWidth.load(std::memory_order_relaxed)) continue;)
            curRulerWidth = m_timeconvertor->rulerWidth.load(std::memory_order_relaxed);
         //   collectBookmarksForDisplay();
            collect();
         //   collectBookmarksForDisplayList();
          //  swapIfNeeded();
        }
    }
    std::atomic_bool toSwap{false};
    std::vector<ShpBookmark> bookmarks;
   // ShpBookmarkVec bookmarksToDisplay;
   // ShpBookmarkVec bookmarksToCompute;


   // SqliteWorker *getSqlworker() const;

    QFileBuffer& getFileWorker();

   // std::atomic<BookmarkVec*> readyBuffer = &bookmarksBuffer[1];
   BookmarkVec* displayBuffer = &bookmarksBuffer[2];
    std::atomic_bool displayBufIsBusy{false};
    std::atomic_bool stale{false};
    QMutex mutex;
private:
    QSharedPointer<TimeConvertor> m_timeconvertor;
    BookmarkVec bookmarksBuffer[3];
    BookmarkVec* computingBuffer = &bookmarksBuffer[0];
    std::vector<Bookmark> testList;

    QVector<QSharedPointer<MultiBookmark>> mVec;

   // SqliteWorker* sqlworker;
    QFileBuffer m_fileworker;

    int curRulerWidth{0};
    void collect(){
       // qDebug() << "recollect";
        auto startCollecting = std::chrono::system_clock::now();
        msecs spread = m_timeconvertor->getUnitingSpread();
        int startCurMbk = 0;
        int startNextMbk = 0;
        int endCurMbk = spread.count() + startCurMbk;

        auto&& vec =  m_fileworker.getBookmarks(m_timeconvertor->left, m_timeconvertor->right, m_timeconvertor);
        *computingBuffer = vec;
        //while(displayBufIsBusy.exchange(true));
        mutex.lock();
       // qDebug() << "collect LOCK";
        auto* temp = displayBuffer;
        displayBuffer = computingBuffer;
        computingBuffer = temp;
        mutex.unlock();
        displayBufIsBusy.store(false);
       // qDebug() << "collect UNLOCK";
        computingBuffer->clear();
       // stale.store(false);
        auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
      //  qDebug() << "timeToReCollectVec" << timeToCollect.count();
    }
    void recollect(){
     /*    qDebug() << "recollect";
         auto startCollecting = std::chrono::system_clock::now();

         QMutableVectorIterator<MultiBookmark> it(*bookmarksToCompute);
         msecs spread = m_timeconvertor->getUnitingSpread();
         auto parse = [=](QMutableVectorIterator<MultiBookmark>& it){
             auto& v = mVec;
             auto b = it;
             auto& cur = it.value();

             auto vec = buf.getBookmarks(msecs(it.value().start), msecs(it.value().end), m_timeconvertor);
             int i = vec.size()-1;
             auto& curM = it.value();
             QVector<Bookmark> splittedDeq;
             for (; i>= 0; i--){
                 auto &curS = vec[i];
                 if (curS.start < curM.end) break;
                 it.insert(MultiBookmark(curS.start, curS.start + spread.count()));
             }
             if (splittedDeq.size() != 0){
                 it.next();
                 it.insert(splittedDeq);
                 it.value()->adjustStart();
                 it.value()->adjustEnd(spread);
             }

             it = b;
             while (it.hasNext()){
                 auto& next = it.next();
                 if (curM->end >= next->start){
                     auto& deq = next->bookmarksDeq;
                     for (int i = 0; i < next->bookmarksDeq.size(); i++){
                         if (deq[i]->start > curM->end) break;
                         curM->bookmarksDeq.push_back(deq.front());
                         deq.pop_front();
                     }

                     if (deq.size() == 0){
                         it.remove();
                     }

                    // curM.bookmarksDeq.push_back(Bookmark(next.start, next.end));
                 }
                 else{
                     break;
                 }
             }
             curM->adjustStart();
             curM->adjustEnd(spread);
             return it;
         };
         while(it.hasNext()){
             it.next();
             parse(it);
         }


         auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
         qDebug() << "timeToReCollectVec" << timeToCollect.count();*/
    }

    void collectBookmarksForDisplay(){
       /* auto startCollecting = std::chrono::system_clock::now();
        msecs unitingSpread = m_timeconvertor->getUnitingSpread();
        mVec.clear();
        mVec.reserve(bookmarks.size());
        for (size_t i = 0; i < bookmarks.size()-1; i++){
            const ShpBookmark& cur = bookmarks[i];
            msecs curUnitingSpread = cur->start + unitingSpread;
            if (curUnitingSpread < bookmarks[i+1]->start){
                mVec.push_back(QSharedPointer<MultiBookmark>::create(cur, cur->end));
                continue;
            }
            QSharedPointer<MultiBookmark> multi = QSharedPointer<MultiBookmark>::create(cur, curUnitingSpread);
            while (curUnitingSpread >= bookmarks[i+1]->start){
                merge(multi, bookmarks[i+1]);
                i++;
                if (i >= bookmarks.size()-1) break;
            }
            multi->adjustEnd(unitingSpread);
            mVec.push_back(multi);
        }
        auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
        qDebug() << "timeToCollectVec" << timeToCollect.count();*/
    }
 /*   void collectBookmarksForDisplayList(){
        testList.clear();
       std::copy( bookmarks.begin(), bookmarks.end(), std::back_inserter( testList ) );
        auto startCollecting = std::chrono::system_clock::now();
        msecs unitingSpread = m_timeconvertor->getUnitingSpread();

        auto end = testList.end()--;
        for (auto b = testList.begin(); b != end; ++b){
            auto&& cur = *b;
            msecs curUnitingSpread = cur->start + unitingSpread;
            auto next = std::next(b);
            if (curUnitingSpread < (*next)->start){
                continue;
            }
            auto multi = QSharedPointer<MultiBookmark>::create(cur, curUnitingSpread);
            while (curUnitingSpread >= (*next)->start){
                merge(multi, std::move(*next));
                next = std::next(next);
                if (next == testList.end()) break;
            }
            testList.erase(b, next);
            testList.insert(b, multi);
            b = next--;
        }

        auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
        qDebug() << "timeToCollectList" << timeToCollect.count();
    }
*/
    void swapIfNeeded(){
        if (!toSwap) return;
      //  auto temp = std::atomic_load(&bookmarksToCompute);

        //while (!std::atomic_compare_exchange_weak(bookmarksToDisplay, bookmarksToCompute, bookmarksToDisplay));
    }
    void split(QVector<MultiBookmark>& vec, int pos, MultiBookmark& m, int oldNew, int endNew){
        QVector<MultiBookmark> splitted;
        //while
    }
    void merge(QVector<MultiBookmark>& vec, int pos, MultiBookmark& m, int endNew){

    }


};

#endif // BOOKMARKMANAGER_H
