#ifndef PALETTE_H
#define PALETTE_H
#include <QColor>
struct Palette{
    QColor rulerBorders{"#ed9121"};
    QColor rulerLines{Qt::black};
    QColor rulerBackground{"#FEFED3"};
    QColor labels{Qt::black};
    QColor singleBookmarksBorders{"#024064"};
    QColor singleBookmarksBackground{60, 179, 192, 100};
    QColor multiBookmarksBorders{"#37782C"};
    QColor multiBookmarksBackground{100, 187, 106, 122};
};


#endif // PALETTE_H
