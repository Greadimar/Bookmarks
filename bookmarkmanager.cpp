#include "bookmarkmanager.h"


bool BookmarkManager::generateBookmarks(int count){
    if (isRunning.load()) return false;
   // sqlworker->startDb();
    auto startCollecting = std::chrono::system_clock::now();
    //sqlworker->generateBookmarks(count, m_timeconvertor);
    this->buf.generateFile(m_timeconvertor, isRunning, count);
    /*     bookmarks.clear();
        bookmarks.reserve(count);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned long> startDis(0, m_timeconvertor->msecsInDay.count());
        std::uniform_int_distribution<unsigned long> durationDis(0, m_timeconvertor->msecsIn3hours.count());

        for (int i = 0; i < count; i++){
            msecs start{startDis(gen)};
            msecs duration{durationDis(gen)};
            bookmarks.push_back(ShpBookmark::create(start, start+duration));
        }
        std::sort(bookmarks.begin(), bookmarks.end(), [=](const ShpBookmark& a, const ShpBookmark& b)->bool{
            return (a->start < b->start);
        });*/

    auto timeToCollect = std::chrono::system_clock::now() - startCollecting;
    collect();
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
    //  start();
    return true;
}

QFileBuffer &BookmarkManager::getBuf()
{
    return buf;
}

//SqliteWorker *BookmarkManager::getSqlworker() const
//{
//    return sqlworker;
//}
