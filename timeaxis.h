#ifndef TIMECONVERTOR_H
#define TIMECONVERTOR_H
#include <atomic>
#include "common.h"
struct TimeInfo{
    const static msecs msecsInDay;
    const static msecs msecsIn3hours;
    const static msecs msecsInhour;

};

class TimeAxis: public QObject{
     Q_OBJECT
signals:
    void minChanged();
    void maxChanged();
public:
    Q_PROPERTY(int min READ getMin WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(int max READ getMax WRITE setMax NOTIFY maxChanged)
    TimeAxis(){};

    const int pxSpreadToUnite{100};
    float hourWidthInPx = 100;



    std::atomic<int> min = 0;//msecs(0);
    std::atomic<int> max = TimeInfo::msecsInDay.count();

    msecs step = TimeInfo::msecsInhour;

    void setMin(int vmin){this->min.store((vmin), std::memory_order_relaxed);}
    void setMax(int vmax){this->max.store((vmax), std::memory_order_relaxed);}
    int getMin(){return min.load(std::memory_order_relaxed);}
    int getMax(){return max.load(std::memory_order_relaxed);}

    void updateHourWidthInPx(){
        msecs dur = getDuration();
   //     float msecsIn1Px = dur.count()/rulerWidth.load(std::memory_order_relaxed);
        hourWidthInPx = TimeInfo::msecsInhour.count() * rulerWidth.load(std::memory_order_relaxed) / dur.count();
    }
    msecs getCentre(){
        return getDuration()/2 + msecs(min.load(std::memory_order_relaxed));
    }
    msecs getDuration(){
        return msecs(max.load(std::memory_order_relaxed)) - msecs(min.load(std::memory_order_relaxed));
    }
    msecs getUnitingSpread(){
        return msecs{(TimeInfo::msecsInDay.count()/rulerWidth.load())*pxSpreadToUnite};
    }
    msecs getMsecFromPxPosition(int xPos, int xShift = 0){
        xPos += xShift;
        return msecs(static_cast<int>((xPos * TimeInfo::msecsInhour.count()) / hourWidthInPx));
    }
    int getPxPosFromMsec(msecs mark){
        return mark.count() * (hourWidthInPx / TimeInfo::msecsInhour.count());
    }
    int getPxPosFromMsec(int mark){
        double pxInMsecs = hourWidthInPx / TimeInfo::msecsInhour.count();
        return mark * pxInMsecs;
    }

    std::atomic<int> rulerWidth{100};
};

#endif // TIMECONVERTOR_H
