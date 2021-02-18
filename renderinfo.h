#ifndef RENDERINFO_H
#define RENDERINFO_H
#include "common.h"
#include <QSize>

struct RenderInfo{

    int rulerTopY{0};
    int rulerBottomY{100};
    const int bookmarksTopMargin = 10;
    const int leftMargin = 5;
    const int rightMargin = 5;
    int bookmarksTopY{110};
    int bookmarksBottomY{130};
    const int bookmarksHeight = 20;
    const int extraTableTopMargin = 5;

    QSize tableSize{};



    //render info
    const msecs renderStep{33}; //~30fps
    std::chrono::system_clock::time_point lastRender;
};


#endif // RENDERINFO_H
