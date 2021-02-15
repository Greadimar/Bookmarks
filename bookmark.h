#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QGraphicsItem>
#include "common.h"
#include <variant>
#include <deque>
class BookmarkPainter;
constexpr int namelength = 6;
struct Bookmark
{

    Bookmark(){};
    Bookmark(const int& start, const int& end) : start(start), end(end){
    }
    Bookmark(const int& start, const int& end, const QString &rname) : start(start), end(end), name(rname)
    {
       // setName(rname);
    }

    virtual ~Bookmark(){}
    //virtual void applyPainter(BookmarkPainter& bp);
    void setName(const QString& bkName){
       // memcpy(name, bkName.data(), namelength);
        name = bkName;
      //  std::cop
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

struct BookmarkZone{
    //    static BookmarkZone constructFromOne(Bookmark bm){
    //       // return {bm.start, bm.end, {bm}, 0};
    //    }
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
