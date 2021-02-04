#ifndef TIMECONVERTOR_H
#define TIMECONVERTOR_H
#include <atomic>
#include "common.h"
class TimeConvertor
{
public:
    TimeConvertor(){};
    const int pxSpreadToUnite{100};
    float hourWidthInPx = 100;
    const std::chrono::milliseconds msecsInDay{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(24))};
    const std::chrono::milliseconds msecsIn3hours{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(3))};
    const std::chrono::milliseconds msecsInhour{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(1))};
    std::atomic<msecs> left = msecs(0);
    std::atomic<msecs> right = msecsInDay;
    msecs getUnitingSpread(){
        return msecs{(msecsInDay.count()/rulerWidth.load(std::memory_order_relaxed))*pxSpreadToUnite};
    }
    msecs getMsecFromPxPosition(int xPos, int xShift = 0){
        xPos += xShift;
        return msecs(static_cast<int>((xPos * msecsInhour.count()) / hourWidthInPx));
    }
    std::atomic<int> rulerWidth{0};
};

#endif // TIMECONVERTOR_H
