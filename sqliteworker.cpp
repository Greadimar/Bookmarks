#include "sqliteworker.h"

#include <QDir>
#include <QTimer>
#include <QDebug>
#include <random>

constexpr bool dbgSqliteWorker = true;
#define DBG if constexpr (dbgSqliteWorker) qDebug

void SqliteWorker::startDb()
{
    int rc = SQLITE_OK;

    rc = sqlite3_open(QDir::toNativeSeparators(
                          QString("%1/bookmarksDb.db")
                          .arg(QDir::currentPath())).toUtf8(),
                      &dbSqlite);

    if (rc != SQLITE_OK){
        QString errmsg = tr("Не удаётся открыть базу данных: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
        emit serviceMsg(errmsg);
        sqlite3_close(dbSqlite);
    }
    char* errmsg;
    sqlite3_exec(dbSqlite, "PRAGMA synchronous = OFF", NULL, NULL, &errmsg);    //optimization?
    //  this option shrinks db
    //  sqlite3_exec(dbSqlite_, "pragma auto_vacuum=full", nullptr, nullptr, nullptr);

    createTable(tableName);

}

void SqliteWorker::closeDb()
{
    sqlite3_close(dbSqlite);
}

bool SqliteWorker::generateBookmarks(const QPointer<TimeAxis>& ta, int count){
    curCount = count;
    dropTable(tableName);
    createTable(tableName);

    auto startGen = std::chrono::system_clock::now();
    //  float chunkRatio = count/1000;
    //if (count == 100000000) chunkRatio = count / 10000;
    //  if (chunkRatio == 0) chunkRatio++;
    float chunkRatio = 24.;
    qDebug() << "count, chunkRaio" << count << chunkRatio;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> startDis(0, TimeInfo::msecsInDay.count());
    //   std::uniform_int_distribution<int> durationDis(0, tc->msecsIn3hours.count());
    // const int msecsInGenerateChunk = TimeInfo::msecsInDay.count() / chunkRatio;

    int it = 0;
    int chunksToGenerate = static_cast<int>(count / chunkRatio);
    if (chunksToGenerate == 0) chunksToGenerate++;
    const int msecsInGenerateChunk =  static_cast<int>(TimeInfo::msecsInDay.count() / chunksToGenerate);
    const int maxBkDuration = 2*TimeInfo::msecsInDay.count() / chunkRatio;
    const int bksInChunk = count/chunksToGenerate;
    //    int curStartMsecs = 0;
    //    int curDurationMsecs = msecsInGenerateChunk;
    int chunksIt = 0;
    int k = 0;
    int curPrg = 0;
    emit sendPrg(0);
    while (it < count){
        //if (!isRunning) break;
        QVector<Bookmark> vec;
        vec.reserve(chunksToGenerate);
        int curChunkEnd = it + bksInChunk;
        if (curChunkEnd > count) curChunkEnd = count;

        int curStartMsecs = chunksIt * msecsInGenerateChunk;
        int curStartLimitMsecs = curStartMsecs + msecsInGenerateChunk;
        if (curStartLimitMsecs > TimeInfo::msecsInDay.count()) curStartLimitMsecs = TimeInfo::msecsInDay.count();
        std::uniform_int_distribution<int> startDis(curStartMsecs, curStartLimitMsecs-1);

        for (; it <  curChunkEnd; it++){
            int&& start = startDis(gen);
            //            if (start < 0){
            //                qDebug() << "smth w w";
            //            }
            std::uniform_int_distribution<int> curEndDis(start, start + maxBkDuration);
            vec.append(Bookmark(start, curEndDis(gen)));
        }
        std::sort(vec.begin(), vec.end(), [=](const Bookmark& a, const Bookmark& b)->bool{
            return (a.start < b.start);
        });
        chunksIt++;

        char quInsertion[255];
        const char* tail{0};
        sqlite3_stmt *pStmt;        //statement handle
        sprintf(quInsertion, "INSERT INTO %s (START_TIME, END_TIME, NAME)  VALUES (?, ?, ?)", tableName);
        beginTransaction();
        int rc = sqlite3_prepare_v2(dbSqlite, quInsertion, 255, &pStmt, &tail);
        if (rc != SQLITE_OK){
            QString errmsg = tr("ошибка во время исполнения: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
            DBG() << errmsg;
            emit serviceMsg(errmsg);
            sqlite3_finalize(pStmt);
            return false;
        }

        for (int j = 0; j < vec.size(); j++, k++){

            sqlite3_bind_int(pStmt, 1, vec[j].start);
            sqlite3_bind_int(pStmt, 2, vec[j].end);
            char name[namelength];
            snprintf(name, 16, "bookmark %d", k);
            //generateStr<namelength>(name);
            sqlite3_bind_text(pStmt, 3, name, namelength, NULL);
            rc = sqlite3_step(pStmt);
            rc = sqlite3_clear_bindings(pStmt);
            rc = sqlite3_reset(pStmt);
            //if (it%1000 == 0)
            int prg = 100*(k/static_cast<double>(count));
            if (prg != curPrg){
                emit sendPrg(100*(k/static_cast<double>(count)));
                curPrg = prg;
            }
        }
        //navMap.insert(curChunkKey, curChunkMap);
        commitTransaction();
        sqlite3_finalize(pStmt);
    }


    //indexing
    createIndexTable();


    emit sendPrg(100);
    return true;

}

void SqliteWorker::createTable(char *table){
    static char create[] = "CREATE TABLE IF NOT EXISTS";
    char quCreateSessionTable[255];
    snprintf(quCreateSessionTable, 255, "%s %s (ID INTEGER PRIMARY KEY NOT NULL, START_TIME INT, END_TIME INT, NAME CHAR(255))", create, table);
    execQuery(quCreateSessionTable);

}
void SqliteWorker::dropTable(char *table)
{
    static char drop[] = "DROP TABLE IF EXISTS";
    char quDropTable[255];
    snprintf(quDropTable, 255, "%s %s", drop, table);
    execQuery(quDropTable);
}

void SqliteWorker::execQuery(char *cmd)
{
    int rc = SQLITE_OK;
    char* err;
    rc = sqlite3_exec(dbSqlite, cmd,
                      nullptr, nullptr, &err);
    qWarning() << err;
    if (rc != SQLITE_OK){
        QString errmsg = tr("Ошибка выполнения запроса: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
        // qWarning() << errmsg << err;
        emit serviceMsg(errmsg);
        sqlite3_close(dbSqlite);
    }
}
void SqliteWorker::beginTransaction()
{
    sqlite3_exec(dbSqlite, "begin transaction",
                 nullptr, nullptr, nullptr);
}

void SqliteWorker::commitTransaction()
{
    sqlite3_exec(dbSqlite, "commit transaction",
                 nullptr, nullptr, nullptr);
}


void SqliteWorker::createIndexTable()
{
    char quCreateIndexTable[255];
    snprintf(quCreateIndexTable, 255, "CREATE INDEX %s ON %s (START_TIME)", indexTableName, tableName);
    execQuery(quCreateIndexTable);

}

QVector<MultiBookmark> SqliteWorker::getMultiBookmarks(const int& start, const int& end, const int mbkDuration)
{
    curStart = start;
    curEnd = end;
    QVector<MultiBookmark> vec;
    int i = start;
    while(i < end){
        int curStart = i;
        int next = i + mbkDuration;
        auto result = getBookmarkZone(curStart, next, mbkDuration);
        if (result.has_value()){
            vec << result.value();
        }
        i = next;
    }
    return vec;

}

QVector<Bookmark> SqliteWorker::getBookmarks(const int &start, const int &end)
{
    int indexStartRow = getRowByStartMark(start);
    int indexEndRow = getRowByEndMark(end);
    char quSelect[255];
    sqlite3_stmt* stmt = nullptr;

    QVector<Bookmark> vec;

    snprintf(quSelect, 255, "SELECT * FROM %s WHERE (ID >= %d AND ID <= %d) LIMIT 17", tableName, indexStartRow, indexEndRow);
    int rc = sqlite3_prepare_v2(dbSqlite, quSelect, 255, &stmt, nullptr);
    if (!checkPrepareReturn(rc)){
        sqlite3_finalize(stmt);
        return vec;
    }
    //int c = stmt->count
    while (sqlite3_step(stmt) == SQLITE_ROW){
        const int startTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::START_TIME));
        const int endTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::END_TIME));
        QString name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<int>(BookmarkCols::NAME))));
        Bookmark bk(startTime, endTime, name);
        vec << bk;
    }
    sqlite3_finalize(stmt);
    return vec;
}
/*
std::optional<MultiBookmark> SqliteWorker::getBookmarkZone(const int& mark, int& next){
    //check the count
    int rc{0};
    char quCheckCount[255];
    snprintf(quCheckCount, 255, "SELECT COUNT(*) FROM %s WHERE (START_TIME >= %d AND START_TIME < %d) LIMIT 101", tableName, mark, next);
    sqlite3_stmt* countStmt = nullptr;
    rc = sqlite3_prepare_v2(dbSqlite, quCheckCount, 255, &countStmt, nullptr);
    if (!checkPrepareReturn(rc)) return std::nullopt;
    sqlite3_step(countStmt);
    int count = sqlite3_column_int(countStmt, 0);
    sqlite3_finalize(countStmt);
    if (count == 0) return std::nullopt;
    else if (count == 1){
        char quSelect[255];

        sqlite3_stmt* stmt = nullptr;
        snprintf(quSelect, 255, "SELECT * FROM %s WHERE (START_TIME >= %d AND START_TIME < %d) LIMIT 101", tableName, mark, next);

        rc = sqlite3_prepare_v2(dbSqlite, quSelect, 255, &stmt, nullptr);
         if (!checkPrepareReturn(rc)) return std::nullopt;
        //int c = stmt->count
        if (sqlite3_step(stmt) == SQLITE_ROW){
            const int startTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::START_TIME));
            const int endTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::END_TIME));
            QString name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<int>(BookmarkCols::NAME))));
            MultiBookmark mbk(startTime, endTime);
            mbk.setName(name);
            sqlite3_finalize(stmt);
            next = endTime;
            return mbk;
        }
        else{
            qWarning() << "smth went wrong in reading bookmark";
            sqlite3_finalize(stmt);
            return std::nullopt;
        }
    }
    else{
        char quSelect[255];
        sqlite3_stmt* stmt = nullptr;
        snprintf(quSelect, 255, "SELECT * FROM %s WHERE START_TIME >= %d AND START_TIME < %d LIMIT 101", tableName, mark, next);
        rc = sqlite3_prepare_v2(dbSqlite, quSelect, 255, &stmt, nullptr);
        if (!checkPrepareReturn(rc)) return std::nullopt;
        MultiBookmark mbk(mark, next);
        QString curName;
        while (sqlite3_step(stmt) == SQLITE_ROW){
            mbk.count++;;
        }
        sqlite3_finalize(stmt);
        return mbk;

    }
}
*/



std::optional<MultiBookmark> SqliteWorker::getBookmarkZone(const int& mark, int& next, const int duration){
    //check the count
    //return std::nullopt;
    int indexStartRow = getRowByStartMark(mark);
    int indexEndRow = getRowByEndMark(next);
    if (indexEndRow == 0 || indexStartRow == 0) return std::nullopt;

    int count = indexEndRow - indexStartRow + 1;

    if (count == 0) return std::nullopt;
    auto bk = getBookmarkByRow(indexStartRow);
    if (!bk.has_value()) return std::nullopt;

    else{
        auto targetMbk = getTargetBookmarkZone(bk.value(), duration);
        if (!targetMbk.has_value()) return std::nullopt;
        next = targetMbk->start + duration;
        return targetMbk;

    }
}

std::optional<MultiBookmark> SqliteWorker::getTargetBookmarkZone(const Bookmark &bk, const int duration)
{
    int indexStartRow = getRowByStartMark(bk.start);
    int indexEndRow = getRowByEndMark(bk.start + duration);
    if (indexEndRow == 0 || indexStartRow == 0) return std::nullopt;

    int count = indexEndRow - indexStartRow + 1;
    if (count == 0) return std::nullopt;

    if (count == 1){
        return MultiBookmark(bk);
    }
    auto lastbk = getBookmarkByRow(indexEndRow);
    if (!lastbk.has_value()) return std::nullopt;
    MultiBookmark mbk(bk.start, lastbk->start);
    mbk.count = count;
    return mbk;
}

int SqliteWorker::getRowByStartMark(const int &mark)
{
    char quGetRow[255];
    snprintf(quGetRow, 255, "SELECT ID FROM %s WHERE (START_TIME >= %d) LIMIT 1", tableName, mark);
    sqlite3_stmt* indexRowStmt = nullptr;
    int rc = sqlite3_prepare_v2(dbSqlite, quGetRow, 255, &indexRowStmt, nullptr);
    if (!checkPrepareReturn(rc)){
        sqlite3_finalize(indexRowStmt);
        return 0;
    }
    int r = sqlite3_step(indexRowStmt);
    if (r != SQLITE_ROW){
        sqlite3_finalize(indexRowStmt);
        return 0;
    }
    int row = sqlite3_column_int(indexRowStmt, 0);
    sqlite3_finalize(indexRowStmt);
    return row;
}

int SqliteWorker::getRowByEndMark(const int &mark)
{
    if (mark > curEnd) return curCount;

    char quGetRow[255];
    snprintf(quGetRow, 255, "SELECT ID FROM %s WHERE (START_TIME > %d) LIMIT 1", tableName, mark);
    sqlite3_stmt* indexRowStmt = nullptr;
    int rc = sqlite3_prepare_v2(dbSqlite, quGetRow, 255, &indexRowStmt, nullptr);
    if (!checkPrepareReturn(rc)){
        sqlite3_finalize(indexRowStmt);
        return 0;
    }
    int r = sqlite3_step(indexRowStmt);
    if (r != SQLITE_ROW){
        sqlite3_finalize(indexRowStmt);
        return curCount;
    }
    int row = sqlite3_column_int(indexRowStmt, 0);
    sqlite3_finalize(indexRowStmt);
    return row - 1;
}

std::optional<Bookmark> SqliteWorker::getBookmarkByRow(const int row)
{
    char quSelect[255];
    sqlite3_stmt* stmt = nullptr;
    snprintf(quSelect, 255, "SELECT * FROM %s WHERE ID = %d", tableName, row);
    int rc = sqlite3_prepare_v2(dbSqlite, quSelect, 255, &stmt, nullptr);
    if (!checkPrepareReturn(rc)){
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    //int c = stmt->count
    if (sqlite3_step(stmt) == SQLITE_ROW){
        const int startTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::START_TIME));
        const int endTime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::END_TIME));
        QString name = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<int>(BookmarkCols::NAME))));
        Bookmark bk(startTime, endTime, name);
        sqlite3_finalize(stmt);
        return bk;
    }
    else{
        qWarning() << "smth went wrong in reading bookmark";
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
}



bool SqliteWorker::checkPrepareReturn(const int &rc)
{
    if (rc != SQLITE_OK){
        QString errmsg = tr("ошибка выполнения запроса: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
        DBG() << errmsg;
        emit serviceMsg(errmsg);
        return false;
    }
    return true;
}

//int SqliteWorker::getIndexedRow(const int mark)
//{
//    auto closest = getLowestKey(start, navMap.keys());
//    if (!closest){
//       // qDebug() << "no big key";
//        return 0;};
//    const auto& chunkMap = navMap.value(closest.value());
//    auto closestInner = getLowestKey(start, chunkMap.keys());
//    if (!closestInner){
//        //qDebug() << "no little key";
//        return 0;
//    };
//    int row = chunkMap.value(closestInner.value());
//}

//std::optional<msecs> SqliteWorker::getLowestKey(msecs val, const QList<msecs> &keys){
//    auto const it = std::lower_bound(keys.begin(), keys.end(), val);
//    if (it == keys.end()) { return std::nullopt; }
//    return *it;
//}
