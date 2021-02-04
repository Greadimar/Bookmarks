#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QGraphicsItem>
#include "common.h"
#include <variant>
#include <deque>
class BookmarkPainter;
constexpr int namelength = 64;
struct Bookmark
{

    Bookmark(){};
    Bookmark(const int& start, const int& end) : start(start), end(end){

    }
    Bookmark(const int& start, const int& end, const QString &rname) : start(start), end(end),
    name(rname.mid(0, namelength).toStdString().data())
    {

    }
    virtual ~Bookmark(){}
    virtual void applyPainter(BookmarkPainter& bp);

    int start{0};
    int end{10000};
    char* name;

private:

};


struct MultiBookmark: public Bookmark{
    MultiBookmark(){}
    MultiBookmark(const int& start, const int& end): Bookmark(start, end), count(1){

    }
    void reset(const int& start, const int& end, const QString &rname){
        this->start = start, this->end = end; count =1; name = rname.mid(0, namelength).toStdString().data();}
    int count = 0;
    void applyPainter(BookmarkPainter &bp) override;
};
struct BookmarkZone{
    static BookmarkZone constructFromOne(Bookmark bm){
        return {bm.start, bm.end, {bm}, 0};
    }
    msecs start{0};
    msecs end{10000};
    std::deque<Bookmark> bmsToSplit;
    int count = 0;
};


using BookmarkVar = std::variant<Bookmark, MultiBookmark>;

class BookmarkPainter{
    const QPainter& m_painter;
public:
    BookmarkPainter( const QPainter& p): m_painter(p) {}
    void paint(const Bookmark& b){
        //implement
    }
    void paint(const MultiBookmark& mb){
        //implement
    }
};



class BookmarkItem: public QGraphicsItem
{
public:
    BookmarkItem(){};
    BookmarkItem(const ShpBookmark& bookmark)  {
    }
    QRectF boundingRect() const override{
       // return QRectF(0,0,)
    }


private:


};


#endif // BOOKMARK_H
