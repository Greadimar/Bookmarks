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
    float dayWidthInPx = hourWidthInPx * 24;
    //int rulerWidth = 100;
    int offsetInPx = 0;
    float zoomRatio = 1;
    int dragOffset = 0;
    int dragOffsetCur = 0;

    std::atomic<int> min = 0;//msecs(0);
    std::atomic<int> max = TimeInfo::msecsInDay.count();

    int step = TimeInfo::msecsInhour.count();

    int stepInPx() {return (step * hourWidthInPx / TimeInfo::msecsInhour.count());}

    void setMin(int vmin){this->min.store((vmin), std::memory_order_relaxed);}
    void setMax(int vmax){this->max.store((vmax), std::memory_order_relaxed);}
    int getMin(){return min.load(std::memory_order_relaxed);}
    int getMax(){return max.load(std::memory_order_relaxed);}




    msecs getCentre(){
        return getVisibleDuration()/2 + msecs(min.load(std::memory_order_relaxed));
    }
    msecs getVisibleDuration(){
        return msecs(max.load(std::memory_order_relaxed)) - msecs(min.load(std::memory_order_relaxed));
    }
    msecs getUnitingSpread(){
        return msecs{(TimeInfo::msecsInDay.count()/rulerWidth.load())*pxSpreadToUnite};
    }

    //mapping coords
//    msecs msecFromPx(int xPos){
//        return msecs(static_cast<int>((xPos * TimeInfo::msecsInhour.count()) / hourWidthInPx));
//    }
    int msecFromPx(int xPos){


        return static_cast<int>((xPos * TimeInfo::msecsInhour.count()) / hourWidthInPx);
    }
    int pxPosFromMsec(msecs mark){
        double pxInMsecs = hourWidthInPx / TimeInfo::msecsInhour.count();
        return mark.count() * pxInMsecs - getMin()*pxInMsecs;
    }
    int pxPosFromMsec(int mark){
        double pxInMsecs = hourWidthInPx / TimeInfo::msecsInhour.count();
        return mark * pxInMsecs - getMin()*pxInMsecs;
    }

    std::atomic<int> rulerWidth{1000};
};

#endif // TIMECONVERTOR_H
