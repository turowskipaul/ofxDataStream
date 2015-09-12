//------------------------------------------
// ofxDataStream
// Copyright (C) 2015 Paul Turowski
//
// for storing, processing streams of data
// with OpenFrameworks
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//------------------------------------------

#pragma once

#include "ofMain.h"

using namespace std;

class ofxDataStream {
    // STORAGE
    int streamSize;
    vector<float> prevVals;
    vector<float> vals;
    vector<float> valsN;
    vector<float> deltaVals;
    vector<bool> triggers;
    vector<bool> bonkVals;
    vector<bool> bonkPrevVals;
    vector<bool> bonks;
    bool isThreshed;
    float thresh;
    bool isDecayingGrowing;
    float decayGrowRatio;
    bool isNormalized;
    ofVec2f valRange;
    bool isClamped;
    float clampLo, clampHi;
    bool isBonked;
    float bonkLo, bonkHi;
    float maxValue;
    float maxValueN;

    // SMOOTHING
    vector<vector<float> > smoothHistos;
    int histoSize;
    float slideUp;
    float slideDown;

    float smooth(int _idx, float _val);

public:
    enum Smoothing_t {
        SMOOTHING_NONE,
        SMOOTHING_ACCUM,
        SMOOTHING_SLIDE
    } smoothingType;

    enum Mean_t {
        MEAN_ARITH,
        MEAN_GEOM,
        MEAN_HARM,
    } meanType;

    ofxDataStream();
    ofxDataStream(int _size);
    void init(int _size);

    void initAccum(int _depth);
    void initSlide(float _sU, float _sD);

    void update(const vector<float>& _vals);
    void update(float _val, int _idx=0);

    void setThresh(float _t);
    void setThreshN(float _tN);
    float getThresh();
    float getThreshN();
    void setDecayGrow(bool _isDG=true, float _ratio=0.99);
    bool getDecayingGrowing();
    void setNormalized(bool _n=true, ofVec2f _range = ofVec2f(0,1));
    void setRange(ofVec2f _range);
    void setRangeLo(int _idx=0);
    void setRangeHi(int _idx=0);
    ofVec2f getRange();

    void setClamp(bool _c=true, float _lo=0, float _hi=1);
    void setBonk(float _hi=0.1, float _lo=0);
    
    float getValue(int _idx=0);
    float getValueN(int _idx=0);
    float getDeltaValue(int _idx=0);
    bool getTrigger(int _idx=0);
    bool getBonk(int _idx=0);
    float getMaxVal();
    float getMaxValN();
    void setMeanType(Mean_t _type);

    const vector<float>& getStream();
    const vector<float>& getStreamN();
    const vector<bool>& getTriggers();
    const vector<float>& getDeltas();
    const vector<bool>& getBonks();
};
