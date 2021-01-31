#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QGraphicsItem>
#include "common.h"
#include <variant>
#include <deque>
class BookmarkPainter;
struct Bookmark
{

    Bookmark(){};
    Bookmark(const msecs& start, const msecs& end) : start(start), end(end){

    }
    virtual ~Bookmark(){}
    virtual void applyPainter(BookmarkPainter& bp);
    QString name;
    msecs start{0};
    msecs end{10000};
private:

};
struct MultiBookmark: public Bookmark{
    MultiBookmark(){

    }
    MultiBookmark(std::deque<ShpBookmark> deq): bookmarksDeq(deq){

    }
    MultiBookmark(const ShpBookmark& bm, const msecs& end):
        Bookmark(bm->start, end){
        bookmarksDeq.push_back(bm);}
    std::deque<ShpBookmark> bookmarksDeq;
    void adjustStart(){
        if (bookmarksDeq.size() != 0){
            start = bookmarksDeq[0]->start;
        }
    }
    void adjustEnd(msecs spread){
        if (bookmarksDeq.size() == 1){
            end = bookmarksDeq[0]->end;
        }
        else{
            end = start + spread;
        }
    }
    void applyPainter(BookmarkPainter &bp) override;
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
