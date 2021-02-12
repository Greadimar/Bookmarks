#include "qfilebuffer.h"
#include <QDir>
#include <QDataStream>
#include <random>
#include <QDebug>
QFileBuffer::QFileBuffer(): file(QDir::currentPath()+"b.dat"),  ds(&file)
, fileB(QDir::currentPath() + "bb.dat"), dsB(&fileB) {

    file.open(QIODevice::ReadWrite);
    file.resize(0);

    fileB.open(QIODevice::ReadWrite);
    fileB.resize(0);
}

void QFileBuffer::generateFile(QSharedPointer<TimeAxis> tc, const std::atomic_bool& isRunning, int count)
{
    auto startGen = std::chrono::system_clock::now();
    const static int chunkRatio = count%1000;
    const int innerChunkRation = chunkRatio%10;
    file.resize(0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> startDis(0, TimeInfo::msecsInDay.count());
    //   std::uniform_int_distribution<int> durationDis(0, tc->msecsIn3hours.count());
    const int msecsInGenerateChunk = TimeInfo::msecsInDay.count() / chunkRatio;

    int it = 0;
    const int chunkToGenerate = std::ceil(count / static_cast<float>(chunkRatio));

    int curStartMsecs = 0;
    int curDurationMsecs = msecsInGenerateChunk;
    int chunksIt = 0;

    int k = 0;
    while (it < count){
        //if (!isRunning) break;
        QVector<Bookmark> vec;
        vec.reserve(chunkToGenerate);
        int curChunkEnd = it + chunkToGenerate;
        if (curChunkEnd > count) curChunkEnd = count;


        int curStartMsecs = chunksIt * msecsInGenerateChunk;
        int curEndMsecs = curStartMsecs + msecsInGenerateChunk;
        if (curEndMsecs > TimeInfo::msecsInDay.count()) curEndMsecs = TimeInfo::msecsInDay.count();
        std::uniform_int_distribution<int> startDis(curStartMsecs, curEndMsecs-1);

        for (; it <  curChunkEnd; it++){
            int&& start = startDis(gen);
            std::uniform_int_distribution<int> curEndDis(start, curEndMsecs);
            vec.append(Bookmark(start, curEndDis(gen)));
        }
        std::sort(vec.begin(), vec.end(), [=](const Bookmark& a, const Bookmark& b)->bool{
            return (a.start < b.start);
        });
        chunksIt++;

        //mapping;
        msecs curChunkKey(vec[0].start);
        QMap <msecs, int> curChunkMap;
        for (int j = 0; j < vec.size(); j++, k++){
            if (j%innerChunkRation == 0)
                curChunkMap.insert(msecs(vec[j].start), k);
            ds << vec[j].start << vec[j].end << vec[j].name;
            emit sendPrg(100*(k/static_cast<double>(count)));
        }
        navMap.insert(curChunkKey, curChunkMap);

    }

    emit sendPrg(100);
}

void QFileBuffer::generateFileB(QSharedPointer<TimeAxis> tc, const std::atomic_bool &isRunning, int count)
{
//    file.seek(0);
//    int curStart{0}; int curEnd{}; char name[namelength];
//    int nextStart{0};
//    for (int i = 0; i < count; i++){
//        MultiBookmark mb;

//        ds >> nextStart;

//        if (curStart > nextStart){

//        }
//    }
}

QVector<MultiBookmark> QFileBuffer::getBookmarks(msecs start, msecs end, QSharedPointer<TimeAxis> ta)
{
    auto closest = getLowestKey(start, navMap.keys());
    if (!closest){
       // qDebug() << "no big key";
        return {};};
    const auto& chunkMap = navMap.value(closest.value());
    auto closestInner = getLowestKey(start, chunkMap.keys());
    if (!closestInner){
        //qDebug() << "no little key";
        return {};
    };
    int row = chunkMap.value(closestInner.value());
    file.seek(row);
   // QVector<Bookmark> singleBkVec;
   // QVector<MultiBookmark> multiBkVec;
    QVector<MultiBookmark> mbVec;
    MultiBookmark* lastMbk;
    //int prevStart, prevEnd; QString prevName;
    int curStart, curEnd; QString name;
    ds >> curStart >> curEnd >> name;
    int endCount = end.count();
    MultiBookmark mbk(curStart, curEnd);
    const static int toSkip = sizeof (int) + 64 * sizeof (char);
    int maxMbkWidth = ta->getUnitingSpread().count();
    int curMbkEnd = mbk.start + maxMbkWidth;
    while(true){
        ds >> curStart >> curEnd >> name;
        if (curStart > end.count() || ds.atEnd()){
            mbVec.append(mbk);
            break;
        }
        if (curStart <= curMbkEnd){
            mbk.count++;
        }
        else{
//            if (mbk.count == 1){
                mbVec.append(mbk);
                mbk.reset(curStart, curEnd, name);
                curMbkEnd = curStart + maxMbkWidth;
//            }
//            else{
//                mbVec.append(mbk);
//                ds >> curEnd >> name;
//                mbk.reset(curStart, curEnd, name);
//                curMbkEnd = curStart + maxMbkWidth;
//            }
        }
    }

    return mbVec;

}

QVector<std::variant<Bookmark, MultiBookmark>> QFileBuffer::getBookmarksVar(msecs start, msecs end, QSharedPointer<TimeAxis> ta)
{
    auto closest = getLowestKey(start, navMap.keys());
    if (!closest){
       // qDebug() << "no big key";
        return {};};
    const auto& chunkMap = navMap.value(closest.value());
    auto closestInner = getLowestKey(start, chunkMap.keys());
    if (!closestInner){
        //qDebug() << "no little key";
        return {};
    };
    int row = chunkMap.value(closestInner.value());
    file.seek(row);
   // QVector<Bookmark> singleBkVec;
   // QVector<MultiBookmark> multiBkVec;
    QVector<std::variant<Bookmark, MultiBookmark>> mbVec;
    MultiBookmark* lastMbk;
    //int prevStart, prevEnd; QString prevName;
    int curStart, curEnd; QString name;
    ds >> curStart >> curEnd >> name;
    int endCount = end.count();
    MultiBookmark mbk(curStart, curEnd);
    const static int toSkip = sizeof (int) + 64 * sizeof (char);
    int maxMbkWidth = ta->getUnitingSpread().count();
    int curMbkEnd = mbk.start + maxMbkWidth;
    while(true){
        ds >> curStart >> curEnd >> name;
        if (curStart > end.count() || ds.atEnd()){
            mbVec.append(mbk.detachVar());
            break;
        }
        if (curStart <= curMbkEnd){
            mbk.count++;
        }
        else{
//            if (mbk.count == 1){
                mbVec.append(mbk.detachVar());
                mbk.reset(curStart, curEnd, name);
                curMbkEnd = curStart + maxMbkWidth;
//            }
//            else{
//                mbVec.append(mbk);
//                ds >> curEnd >> name;
//                mbk.reset(curStart, curEnd, name);
//                curMbkEnd = curStart + maxMbkWidth;
//            }
        }
    }

    return mbVec;

}

