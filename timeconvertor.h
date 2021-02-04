#ifndef TIMECONVERTOR_H
#define TIMECONVERTOR_H
#include <atomic>
#include "common.h"
class TimeConvertor
{
public:
    TimeConvertor(){};
    const int pxSpreadToUnite{100};
    const std::chrono::milliseconds msecsInDay{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(24))};
    const std::chrono::milliseconds msecsIn3hours{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(3))};
    std::atomic<msecs> left = msecs(0);
    std::atomic<msecs> right = msecsInDay;
    msecs getUnitingSpread(){
        return msecs{(msecsInDay.count()/rulerWidth.load(std::memory_order_relaxed))*pxSpreadToUnite};
    }
    std::atomic<int> rulerWidth{0};
};

#endif // TIMECONVERTOR_H
