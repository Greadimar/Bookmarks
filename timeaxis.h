#ifndef TIMECONVERTOR_H
#define TIMECONVERTOR_H
#include <atomic>
#include "common.h"
#include <QDataStream>
struct TimeInfo{
    const static msecs msecsInDay;
    const static msecs msecsIn3hours;
    const static msecs msecsInhour;
};

struct AxisInfo{    //pixels values
    AxisInfo(){}
    AxisInfo (float hourw, float dayw, float stepw, int dragOff, int dragOffCur = 0):
        hourWidthInPx(hourw), dayWidthInPx(dayw), stepInPx(stepw), dragOffset(dragOff), dragOffsetCur(dragOffCur){}
    float hourWidthInPx = 100;
    float dayWidthInPx = hourWidthInPx * 24;
    float stepInPx = hourWidthInPx;

    int dragOffset = 0;
    int dragOffsetCur = 0;
};
inline QDataStream &operator >>(QDataStream &ds, AxisInfo &ai){
    ds  >> ai.hourWidthInPx >> ai.dayWidthInPx >> ai.stepInPx >> ai.dragOffset >> ai.dragOffsetCur;
    return ds;
}
inline QDataStream &operator <<(QDataStream &ds, const AxisInfo &ai){
    ds << ai.hourWidthInPx << ai.dayWidthInPx << ai.stepInPx << ai.dragOffset << ai.dragOffsetCur;
    return ds;
}
inline bool operator==(const AxisInfo& lhs, const AxisInfo& rhs)
{
    return (lhs.stepInPx == rhs.stepInPx &&
            lhs.dragOffset == rhs.dragOffset &&
            lhs.dayWidthInPx == rhs.dayWidthInPx &&
            lhs.dragOffsetCur == rhs.dragOffsetCur &&
            lhs.hourWidthInPx == rhs.hourWidthInPx);
}
inline bool operator!=(const AxisInfo& lhs, const AxisInfo& rhs)
{
    return !(lhs == rhs);
}
Q_DECLARE_METATYPE(AxisInfo)

class TimeAxis: public QObject{
     Q_OBJECT

signals:
    void minChanged();
    void maxChanged();
public:
    Q_PROPERTY(int min READ getMin WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(int max READ getMax WRITE setMax NOTIFY maxChanged)
//    Q_PROPERTY(float hourWidth MEMBER hourWidthInPx)
//    Q_PROPERTY(float dayWidth MEMBER dayWidthInPx)
//    Q_PROPERTY(float step MEMBER stepInPx)
    Q_PROPERTY(int zoomOffMsecs READ getZoomOffsetMsecs WRITE setZoomOffsetMsecs)
    Q_PROPERTY(AxisInfo axisInfo READ getAi WRITE setAi)

    TimeAxis(){
        qRegisterMetaType<AxisInfo>("AxisInfo");
    };

    const int pxSpreadToUnite{100};


    int step = TimeInfo::msecsInhour.count();



    msecs getCentre(){
        return getVisibleDuration()/2 + msecs(m_min.load(std::memory_order_relaxed));
    }
    msecs getVisibleDuration(){
        return msecs(m_max.load(std::memory_order_relaxed)) - msecs(m_min.load(std::memory_order_relaxed));
    }
    int getUnitingSpread(){
        return ((getMax() - getMin())/rulerWidth.load())*pxSpreadToUnite;
    }

    //mapping coords

    int msecFromPx(int xPos){
        return static_cast<int>((xPos * TimeInfo::msecsInhour.count()) / m_ai.hourWidthInPx);
    }
    int pxPosFromMsec(msecs mark){
        double pxInMsec = m_ai.hourWidthInPx / TimeInfo::msecsInhour.count();
        return mark.count() * pxInMsec - getMin()*pxInMsec;
    }
    int pxPosFromMsec(int mark){
        double pxInMsec = m_ai.hourWidthInPx / TimeInfo::msecsInhour.count();
        return mark * pxInMsec - getMin()*pxInMsec;
    }

    std::atomic<int> rulerWidth{1000};


    //getters and setters
    int getZoomOffsetMsecs() const;
    void setZoomOffsetMsecs(int zoomOffsetMsecs);
    void setMin(int vmin){this->m_min.store((vmin), std::memory_order_relaxed);}
    void setMax(int vmax){this->m_max.store((vmax), std::memory_order_relaxed);}
    int getMin(){return m_min.load(std::memory_order_relaxed);}
    int getMax(){return m_max.load(std::memory_order_relaxed);}
    AxisInfo getAi() const;
    void setAi(const AxisInfo &ai);
private:
    std::atomic<int> m_min = 0;//msecs(0);
    std::atomic<int> m_max = TimeInfo::msecsInDay.count();
    int m_zoomOffsetMsecs = 0;

    //pixels
//    float m_hourWidthInPx = 100;
//    float m_dayWidthInPx = m_hourWidthInPx * 24;
//    float m_stepInPx = m_hourWidthInPx;

//    int m_dragOffset = 0;
//    int m_dragOffsetCur = 0;

    AxisInfo m_ai;

};

#endif // TIMECONVERTOR_H
