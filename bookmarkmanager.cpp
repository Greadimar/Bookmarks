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
    auto bks = sqlworker->getMultiBookmarks(m_ta->getMin(), m_ta->getMax(), m_ta->getUnitingSpread());


    auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
    qDebug() << "time" << timeToCollect.count() << duration_cast<std::chrono::seconds>(timeToCollect).count();
}

bool BookmarkManager::generateBookmarks(int count){
     sqlworker->startDb();
    auto startCollecting = std::chrono::system_clock::now();
    sqlworker->generateBookmarks(m_ta, count);


    auto timeToCollect = std::chrono::system_clock::now() - startCollecting;


     qDebug() << "timeTognrt" << timeToCollect.count();

     bufferIsReady = false;
     tableBufferIsReady = false;
     start();
    return true;
}


//SqliteWorker *BookmarkManager::getSqlworker() const
//{
//    return sqlworker;
//}
