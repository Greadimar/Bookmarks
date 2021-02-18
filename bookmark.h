#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QGraphicsItem>
#include "common.h"
#include <variant>
#include <deque>
constexpr int namelength = 16;
struct Bookmark
{
    Bookmark(){};
    Bookmark(const int& start, const int& end) : start(start), end(end){
    }
    Bookmark(const int& start, const int& end, const QString &rname) : start(start), end(end), name(rname)
    {
    }

    virtual ~Bookmark(){}
    void setName(const QString& bkName){
        name = bkName;
    }
    int start{0};
    int end{10000};
    QString name{""};

private:

};


struct MultiBookmark: public Bookmark{
    MultiBookmark(){}
    MultiBookmark(const Bookmark& bk): Bookmark(bk.start, bk.end, bk.name), count(1){}
    MultiBookmark(const int& start, const int& end): Bookmark(start, end), count(1){
    }
    //QVector<int> forSeek;
    void reset(const int& start, const int& end, const QString &rname){
        this->start = start;
        this->end = end;
        count =1;
        setName(rname);
    }
    int count = 0;
   Bookmark& detachVar(){
        return (count > 1)? *this : static_cast<Bookmark&>(*this);
    }

    //  void applyPainter(BookmarkPainter &bp) override;
};





#endif // BOOKMARK_H
