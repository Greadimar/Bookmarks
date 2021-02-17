#ifndef SQLITEWORKER_H
#define SQLITEWORKER_H
#include "sqlite3.h"
#include "bookmark.h"
#include "timeaxis.h"
#include <optional>
#include <QObject>
#include <QTime>
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
    SqliteWorker(std::atomic_bool& isRunning, QObject* parent): QObject(parent), isRunning(isRunning){
        srand(QTime::currentTime().msecsSinceStartOfDay());
    };
    void startDb();
    void closeDb();
    bool generateBookmarks(const QSharedPointer<TimeAxis> &tc, int count);

    QVector<MultiBookmark> getMultiBookmarks(const int &start, const int &end, const int mbkDuration);
    QVector<Bookmark> getBookmarks(const int& start, const int & end);
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

    int curCount{0};
    int curStart{0};
    int curEnd{0};

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


   // std::optional<MultiBookmark> getBookmarkZone(const int &mark, int &next);
    std::optional<MultiBookmark> getBookmarkZone(const int &mark, int &next, const int duration);
        std::optional<MultiBookmark> getTargetBookmarkZone(const Bookmark& bk, const int duration);
    int getRowByStartMark(const int& mark);
        int getRowByEndMark(const int& mark);
    std::optional<Bookmark> getBookmarkByRow(const int row);
    bool checkPrepareReturn(const int& rc);

    template<int size> void generateStr(char* str){
        static const char set[] =
        "0123456789"
      //  "!@#$%^&*"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        static const int len = sizeof(set) - 1;
        for (int i = 0; i < size; i++){
            str[i] = set[rand() % len];
        }
    }


};

#endif // SQLITEWORKER_H
