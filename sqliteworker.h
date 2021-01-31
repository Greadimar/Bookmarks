#ifndef SQLITEWORKER_H
#define SQLITEWORKER_H
#include "sqlite3.h"
#include "bookmark.h"
#include "timeconvertor.h"
#include <QObject>

#include <queue>
//constexpr bool dbgSqliteWorker = false;
//#define DBG if constexpr (dbgSqliteWorker) qDebug
class SqliteWorker : public QObject
{
    Q_OBJECT
public:
    enum class LoggingMode{
        dbOnHard,
        dbOnRAM,
    };
    SqliteWorker(){};
    void startDb();
    void generateBookmarks(int count, const QSharedPointer<TimeConvertor> &tc);
    QVector<Bookmark> getBookmarks(msecs start, msecs end);

public slots:


signals:
    void serviceMsg(QString msg);
    void sendPrg(int prg);
private:
    LoggingMode m_mode;
    enum SessionStatus{
        idle, work
    };
    enum class BookmarkCols: int{
        ID = 0, START_TIME = 1, END_TIME = 2, NAME = 3
    };
    char unsortedTableName[18] = "bookmarksUTable";        //unsorted temporary table
    char tableName[17] = "bookmarksTable";          //sorted table


    SessionStatus sessionStatus{SessionStatus::idle};


    sqlite3* dbSqlite{nullptr};
    void initDb();
    QString filename;
    void createTable(char* table);
    void dropTable(char *table);
    using timePoint = std::chrono::time_point<std::chrono::system_clock>;
    timePoint waitNotesResetTp{std::chrono::system_clock::now()};
    std::chrono::milliseconds maxWaitNotesTimeout{2000};

  //  void writeCurNotes();
    void execQuery(char* cmd);
    void beginTransaction();
    void commitTransaction();
    void createTables();
    MultiBookmark getMultibookmarkIn(const msecs& start, const msecs& end);
    bool checkDb();
    int getLastId();
    int getLastMsgId();
    /*  static int idGetter(void *, int, char **, char **);*/

    using CallbackDataGetter = int (*)(void*, int, char **, char **);
    QString addInvCommas(const QString& str);

};

#endif // SQLITEWORKER_H
