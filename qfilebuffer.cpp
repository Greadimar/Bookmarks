#include "qfilebuffer.h"
#include <QDir>
#include <QDataStream>
#include <random>
#include <QDebug>
QFileBuffer::QFileBuffer(): file(QDir::currentPath()+"b.dat"),  ds(&file)
{

    file.open(QIODevice::ReadWrite);
    file.resize(0);
}

void QFileBuffer::generateFile(QSharedPointer<TimeConvertor> tc, const std::atomic_bool& isRunning, int count)
{
    auto startGen = std::chrono::system_clock::now();
    const static int chunkRatio = 1000;
    const int innerChunkRation = chunkRatio/10 + 1;


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> startDis(0, tc->msecsInDay.count());
    //   std::uniform_int_distribution<int> durationDis(0, tc->msecsIn3hours.count());
    const int msecsInGenerateChunk = tc->msecsInDay.count() / chunkRatio;

    int it = 0;
    const int chunkToGenerate = count / chunkRatio;

    int curStartMsecs = 0;
    int curDurationMsecs = msecsInGenerateChunk;
    int chunksIt = 0;

    int k = 0;
    while (it < count){
        if (!isRunning) break;
        QVector<Bookmark> vec;
        vec.reserve(chunkToGenerate);
        int curChunkEnd = it + chunkToGenerate;
        if (curChunkEnd > count) curChunkEnd = count;


        int curStartMsecs = chunksIt * msecsInGenerateChunk;
        int curEndMsecs = curStartMsecs + msecsInGenerateChunk;
        if (curEndMsecs > tc->msecsInDay.count()) curEndMsecs = tc->msecsInDay.count();
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
            if (j%innerChunkRation) curChunkMap.insert(msecs(vec[j].start), k);
            ds << vec[j].start << vec[j].end << vec[j].name;
            emit sendPrg(100*(k/static_cast<double>(count)));
        }
        navMap.insert(curChunkKey, curChunkMap);

        emit sendPrg(100);
    }
}

QVector<std::variant<Bookmark, MultiBookmark> > QFileBuffer::getBookmarks(msecs start, msecs end, QSharedPointer<TimeConvertor> tc)
{
    auto closest = getLowestKey(start, navMap.keys());
    if (!closest){qDebug() << "no big key"; return {};};
    const auto& chunkMap = navMap.value(closest.value());
    auto closestInner = getLowestKey(start, chunkMap.keys());
    if (!closestInner){qDebug() << "no big key"; return {};};
    int row = chunkMap.value(closestInner.value());
    file.seek(row);
    QVector<std::variant<Bookmark, MultiBookmark>> mbVec;
    MultiBookmark* lastMbk;
    int prevStart, prevEnd; QString prevName;
    int curStart, curEnd; QString name;
    ds >> curStart >> curEnd >> name;
    int endCount = end.count();
    MultiBookmark mbk(prevStart, prevEnd);
    const static int toSkip = sizeof (int) + 64 * sizeof (char);
    int maxMbkWidth = tc->getUnitingSpread().count();
    int curMbkEnd = mbk.start + maxMbkWidth;
    while(true){
        ds >> curStart;
        if (curStart > curEnd) break;
        if (curStart <= curMbkEnd){
            ds.skipRawData(toSkip);
            mbk.count++;
        }
        else{
            if (mbk.count == 1){
                mbVec.append(static_cast<Bookmark&>(mbk));
                ds >> curEnd >> name;
                mbk.reset(curStart, curEnd, name);
                curMbkEnd = curStart + maxMbkWidth;
            }
            else{
                mbVec.append(mbk);
                ds >> curEnd >> name;
                mbk.reset(curStart, curEnd, name);
                curMbkEnd = curStart + maxMbkWidth;
            }
        }
    }
    mbVec.append(mbk);
    return mbVec;

}
