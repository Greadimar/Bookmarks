#include "sqliteworker.h"

#include <QDir>
#include <QTimer>
#include <QDebug>
#include <random>
//SqliteWorker::SqliteWorker()
//{

//}
constexpr bool dbgSqliteWorker = false;
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
    //createTable();



}

void SqliteWorker::generateBookmarks(int count,  const QSharedPointer<TimeConvertor>& tc){
    auto startGen = std::chrono::system_clock::now();
    const static int chunkRatio = 100;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> startDis(0, tc->msecsInDay.count());
 //   std::uniform_int_distribution<int> durationDis(0, tc->msecsIn3hours.count());
    const int msecsInGenerateChunk = tc->msecsInDay.count() / chunkRatio;
    int i = 0;

    dropTable(unsortedTableName);
    createTable(unsortedTableName);





    int it = 0;
    const int chunkToGenerate = count / chunkRatio;

    int curStartMsecs = 0;
    int curDurationMsecs = msecsInGenerateChunk;
    int chunksIt = 0;
    while (it < count){
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
            vec.append(Bookmark(msecs(start), msecs(curEndDis(gen))));
        }
        std::sort(vec.begin(), vec.end(), [=](const Bookmark& a, const Bookmark& b)->bool{
            return (a.start < b.start);
        });
        chunksIt++;

        char quInsertion[255];
        const char* tail{0};
        sqlite3_stmt *pStmt;        //statement handle
        sprintf(quInsertion, "INSERT INTO %s (START_TIME, END_TIME, NAME)  VALUES (?, ?, ?)", unsortedTableName);
        beginTransaction();
        int rc = sqlite3_prepare_v2(dbSqlite, quInsertion, 255, &pStmt, &tail);
        if (rc != SQLITE_OK){
            QString errmsg = tr("ошибка во время исполнения: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
            DBG() << errmsg;
            emit serviceMsg(errmsg);
            return;
        }
        for (int j = 0; j < vec.size(); j++){
            sqlite3_bind_int(pStmt, 1, vec[j].start.count());
            sqlite3_bind_int(pStmt, 2, vec[j].end.count());
            char name[16];
            snprintf(name, 16, "name %d", j);
            sqlite3_bind_text(pStmt, 3, name, strlen(name), NULL);
            rc = sqlite3_step(pStmt);
            rc = sqlite3_clear_bindings(pStmt);
            rc = sqlite3_reset(pStmt);
            //if (it%1000 == 0)
                emit sendPrg(100*(it/static_cast<double>(count)));
        }
        commitTransaction();
        sqlite3_finalize(pStmt);
      //  emit sendPrg(100);
    }

    //sorting
//    createTable(tableName);
//    emit sendPrg(0);
//    static char insert[] = "INSERT INTO";
//    char quDropTable[255];
//    snprintf(quDropTable, 255, "%s %s (START_TIME, END_TIME, NAME) SELECT START_TIME, END_TIME, NAME FROM %s ORDER BY START_TIME", insert, tableName, unsortedTableName);
//    execQuery(quDropTable);
//    dropTable(unsortedTableName);
//    emit sendPrg(100);
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

MultiBookmark SqliteWorker::getMultibookmarkIn(const msecs& start, const msecs& end){
      char quSelect[255];
      int rc = 0;
      sqlite3_stmt* stmt = nullptr;
      snprintf(quSelect, 255, "SELECT * FROM %s WHERE START_TIME >= %d AND START_TIME < %d", tableName, start.count(), end.count());

      rc = sqlite3_prepare_v2(dbSqlite, quSelect, 255, &stmt, nullptr);
      if (rc != SQLITE_OK){
          QString errmsg = tr("ошибка выполнения запроса: %1_%2").arg(rc).arg(sqlite3_errmsg(dbSqlite));
          DBG() << errmsg;
          emit serviceMsg(errmsg);
          return MultiBookmark();
      }
      int rstep = SQLITE_ROW;
      //int c = stmt->count
      while (true){
//          rstep = sqlite3_step(stmt);
//          if (rstep != SQLITE_ROW) break;
//          const int stime = sqlite3_column_int(stmt, static_cast<int>(BookmarkCols::START_TIME));
//          int timestrsize = sqlite3_column_bytes(stmt, static_cast<int>(DataTableCols::TIME));
//          QString timeStr = QString::fromUtf8(reinterpret_cast<const char*>(time), timestrsize);
//          NetType netType = static_cast<NetType>(sqlite3_column_int(stmt, static_cast<int>(DataTableCols::NET_TYPE)));
//          const uint msgtype = (sqlite3_column_int(stmt, static_cast<int>(DataTableCols::MSG_TYPE)));
//          const char* data = static_cast<const char*>(sqlite3_column_blob(stmt, static_cast<int>(DataTableCols::MSG)));
//          int datasize = (sqlite3_column_bytes(stmt, static_cast<int>(DataTableCols::MSG)));
//          res << MsgImagesFabric::createMsgImage(netType, msgtype, data, datasize);
      }
      sqlite3_finalize(stmt);
}
