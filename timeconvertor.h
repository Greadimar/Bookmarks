#ifndef TIMECONVERTOR_H
#define TIMECONVERTOR_H
#include <atomic>
#include "common.h"
class TimeConvertor
{
public:
    TimeConvertor(){};
   // msecs msecsFromPx(const int& px){

 //   }
    //msecs getCurM
    msecs getUnitingSpread(){
        return msecs{(msecsInDay.count()/rulerWidth.load(std::memory_order_relaxed))*pxSpreadToUnite};
    }
    std::atomic<int> rulerWidth{0};
    const int pxSpreadToUnite{100};
    const std::chrono::milliseconds msecsInDay{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(24))};
    const std::chrono::milliseconds msecsIn3hours{std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(3))};
};

#endif // TIMECONVERTOR_H
