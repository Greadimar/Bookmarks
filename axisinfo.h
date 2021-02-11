#ifndef AXISINFO_H
#define AXISINFO_H
#include <QObject>
#include "common.h"
class AxisInfo: public QObject{
public:
    AxisInfo(){
    }
    int minMsecs{0};
    int maxMsecs{0};
    double zoom{1.};
    int shift{0};

    int mInversia{1}; // -1
 /*   void setMin(double min){
        if(std::isnan(min) || std::isinf(min)){
            aDebug() << "Warning Min" << "X:" << min;
            return;
        }
        if(qFuzzyCompare(mMax,min)){
            aDebug() << "Warning Min == Max" << mMin << mMax;
            return;
        }
        if(min > mMax){
            aDebug() << "Warning Min > Max" << min << mMax;
            auto s = mMax;
            mMax = min;
            mMin = s;
            return;
        }
        mMin = min;
    }
    void setMax(double max){
        if(std::isnan(max) || std::isinf(max)){
            aDebug() << "Warning Max" << "X:" << max;
            return;
        }
        if(qFuzzyCompare(max,mMin)){
            aDebug() << "Warning Min == Max" << mMin << max;
            return;
        }
        if(mMin > max){
            aDebug() << "Warning Min >= Max" << mMin << max;
            auto s = mMin;
            mMin = max;
            mMax = s;
            return;
        }
        mMax = max;
    }
    void setRange(double min, double max){
        if(qFuzzyCompare(max,min)){
            aDebug() << "Warning Min == Max" << min << max;
            return;
        }
        if(min > max){
            aDebug() << "Warning Min > Max" << min << max;
            mMax = min;
            mMin = max;
            return;
        }
        mMax = max;
        mMin = min;
    }
    double min() const
    {

        return mMin;
    }
    double max() const
    {

        return mMax;
    }
    double width(){
        return mMax-mMin;
    }
    double shift() const
    {
        return mShift;
    }
    void setShift(double shift)
    {
        mShift = shift;
    }
    double modifer() const
    {
        return mModifer;
    }
    void setModifer(double modifer)
    {
        mModifer = modifer;
    }
    double getAbs(const double &val){
        return (val+shift())*modifer();
    }
    void setInversia(const bool &inv){
        mInversia = inv ? -1 : 1;
    }
    bool inversia() const{
        return mInversia < 0;
    }
    friend bool operator== (const Axis &a1, const Axis &a2){
        return (qFuzzyCompare(a1.mMin, a2.mMin) &&
                qFuzzyCompare(a1.mMax, a2.mMax) &&
                qFuzzyCompare(a1.mShift, a2.mShift) &&
                qFuzzyCompare(a1.mModifer, a2.mModifer));
    }*/
};
#endif // AXISINFO_H
