#ifndef SQLITEWORKER_H
#define SQLITEWORKER_H
#include "sqlite3.h"
#include "bookmark.h"
#include "timeaxis.h"
#include <optional>
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
    SqliteWorker(std::atomic_bool& isRunning): isRunning(isRunning){};
    void startDb();
    void closeDb();
    void generateBookmarks(int count, const QSharedPointer<TimeAxis> &tc);

    QVector<MultiBookmark> getBookmarks(const int &start, const int &end, const int mbkDuration);
signals:
    void sendPrg(int);
    void serviceMsg(QString msg);
private:
    std::atomic_bool& isRunning;
    LoggingMode m_mode;
    enum SessionStatus{
        idle, work
    };
    enum class BookmarkCols: int{
        ID = 0, START_TIME = 1, END_TIME = 2, NAME = 3
    };
    char tableName[16] = "bookmarksTable";          //sorted table
    char indexTableName[16] = "indexTable";
    QMap<msecs, QMap<msecs, int>> navMap;

    SessionStatus sessionStatus{SessionStatus::idle};


    sqlite3* dbSqlite{nullptr};
    void initDb();
    QString filename;
    void createTable(char* table);
    void createIndexTable();
    void dropTable(char *table);
    using timePoint = std::chrono::time_point<std::chrono::system_clock>;
    timePoint waitNotesResetTp{std::chrono::system_clock::now()};
    std::chrono::milliseconds maxWaitNotesTimeout{2000};

  //  void writeCurNotes();
    void execQuery(char* cmd);
    void beginTransaction();
    void commitTransaction();
    void createTables();


    std::optional<MultiBookmark> getBookmarkZone(const int &mark, int &next);
        std::optional<MultiBookmark> getBookmarkZoneSmart(const int &mark, int &next);
    int getRowByStartMark(const int& mark);
        int getRowByEndMark(const int& mark);
    bool checkPrepareReturn(const int& rc);

//    int getIndexedRow(const int mark);
//    std::optional<msecs> getLowestKey(msecs val, const QList<msecs>& keys);
    /*  static int idGetter(void *, int, char **, char **);*/

//    using CallbackDataGetter = int (*)(void*, int, char **, char **);
//    QString addInvCommas(const QString& str);


};

#endif // SQLITEWORKER_H
