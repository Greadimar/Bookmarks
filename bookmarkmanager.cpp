#include "bookmarkmanager.h"


BookmarkManager::BookmarkManager(QSharedPointer<TimeAxis> &timeAxis): QObject(nullptr),

    m_ta(timeAxis)
{
    sqlworker = new SqliteWorker(isRunning, this);
    connect(sqlworker, &SqliteWorker::sendPrg, this, &BookmarkManager::sendPrg);
    connect(sqlworker, &SqliteWorker::serviceMsg, this, &BookmarkManager::serviceMsg);
}

bool BookmarkManager::testGet()
{
    auto startCollecting = std::chrono::system_clock::now();
    //sqlworker->generateBookmarks(count, m_timeconvertor);
    auto bks = sqlworker->getBookmarks(m_ta->getMin(), m_ta->getMax(), m_ta->getUnitingSpread());


    auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
    qDebug() << "time" << timeToCollect.count() << duration_cast<std::chrono::seconds>(timeToCollect).count();
}

bool BookmarkManager::generateBookmarks(int count){
   // if (isRunning.load()) return false;

     sqlworker->startDb();
    auto startCollecting = std::chrono::system_clock::now();
    //sqlworker->generateBookmarks(count, m_timeconvertor);
    //sqlworker->generateBookmarks(m_ta, count);


    auto timeToCollect = std::chrono::system_clock::now() - startCollecting;

   // collect();
    qDebug() << "timeTognrt" << timeToCollect.count();
    //        mVec.resize(bookmarks.size());
    //         msecs unitingSpread = m_timeconvertor->getUnitingSpread();
    //        for (size_t i = 0; i < bookmarks.size(); i++){
    //            const Bookmark& cur = bookmarks[i];
    //            msecs curMultiFinish = cur.start + unitingSpread;
    //            mVec[i] = MultiBookmark(cur, curMultiFinish);
    //            mVec[i].adjustStart();
    //            mVec[i].adjustEnd(unitingSpread);

    //        }
    //  collectBookmarksForDisplay();
    //  mVec.squeeze();
     stale = false;
     start();
    return true;
}


//SqliteWorker *BookmarkManager::getSqlworker() const
//{
//    return sqlworker;
//}
