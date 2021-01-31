#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H
#include "bookmark.h"
#include "timeconvertor.h"
#include "sqliteworker.h"
#include <QThread>
#include <QDebug>
#include <QObject>


using BookmarkList = std::vector<Bookmark>;
class BookmarkManager: public QObject
{
Q_OBJECT
public:
    std::atomic_bool isRunning{false};

    BookmarkManager(QSharedPointer<TimeConvertor>& convertor): QObject(nullptr),

        bookmarksToDisplay(&bookmarksBuffer1),
        bookmarksToCompute(&bookmarksBuffer2),
        m_timeconvertor(convertor)
    {
        sqlworker = new SqliteWorker;
    };
    void stop(){
        isRunning.store(false);
    }
    Q_INVOKABLE bool generateBookmarks(int count);
    void start(){
        isRunning = true;
        while (isRunning){
            QThread::sleep(0);

           // if (curRulerWidth == m_timeconvertor.rulerWidth.load(std::memory_order_relaxed)) continue;)
            curRulerWidth = m_timeconvertor->rulerWidth.load(std::memory_order_relaxed);
         //   collectBookmarksForDisplay();
            recollect();
         //   collectBookmarksForDisplayList();
            swapIfNeeded();
        }
    }
    std::atomic_bool toSwap{false};
    std::vector<ShpBookmark> bookmarks;
    BookmarkList* bookmarksToDisplay;
    BookmarkList* bookmarksToCompute;


    SqliteWorker *getSqlworker() const;

private:
    QSharedPointer<TimeConvertor> m_timeconvertor;
    BookmarkList bookmarksBuffer1;
    BookmarkList bookmarksBuffer2;

    std::vector<Bookmark> testList;

    QVector<QSharedPointer<MultiBookmark>> mVec;

    SqliteWorker* sqlworker;

    int curRulerWidth{0};

    void recollect(){
         qDebug() << "recollect";
         auto startCollecting = std::chrono::system_clock::now();

         QMutableVectorIterator<QSharedPointer<MultiBookmark>> it(mVec);
         msecs spread = m_timeconvertor->getUnitingSpread();
         auto parse = [=](QMutableVectorIterator<QSharedPointer<MultiBookmark>>& it){
             auto& v = mVec;
             auto b = it;
             int i = it.value()->bookmarksDeq.size() - 1;
             auto& curM = it.value();
             std::deque<ShpBookmark> splittedDeq;
             for (; i>= 0; i--){
                 auto &curS = curM->bookmarksDeq[i];
                 if (curS->start < curM->end) break;
                 splittedDeq.push_back(curS);
                 it.insert(QSharedPointer<MultiBookmark>::create(curS, curS->start + spread));
             }
             if (splittedDeq.size() != 0){
                 it.next();
                 it.insert(QSharedPointer<MultiBookmark>::create(splittedDeq));
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
         qDebug() << "timeToReCollectVec" << timeToCollect.count();
    }


    void collectBookmarksForDisplay(){
        auto startCollecting = std::chrono::system_clock::now();
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
        qDebug() << "timeToCollectVec" << timeToCollect.count();
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
        auto temp = bookmarksToCompute;
        bookmarksToCompute = bookmarksToDisplay;
        bookmarksToDisplay = temp;
//        auto&& temp = bookmarksToDisplay.fetchAndStoreOrdered(bookmarksToCompute.loadRelaxed());
//        bookmarksToCompute.fetchAndStoreOrdered(temp);
        toSwap.store(false, std::memory_order_seq_cst);
    }
    void merge(QSharedPointer<MultiBookmark>& m, const ShpBookmark& b){
        m->bookmarksDeq.push_back(b);
    }


};

#endif // BOOKMARKMANAGER_H
