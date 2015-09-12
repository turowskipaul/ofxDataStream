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

#include "ofxDataStream.h"

ofxDataStream::ofxDataStream() {
    // ofxDataStream(1);
    // note: delegating constructors only works in c++ 11
    // which doesn't seem to implemented in OF yet...
    init(1);
}

ofxDataStream::ofxDataStream(int _size){
    init(_size);
}

void ofxDataStream::init(int _size) {
    if (_size < 1) {
        ofLogError("ofxDataStream") << "ofxDataStream(): size must be at least 1";
        return;
    }
    for (int v=0; v<_size; v++) {
        prevVals.push_back(0);
        vals.push_back(0);
        valsN.push_back(0);
        deltaVals.push_back(0);
        triggers.push_back(false);
    }
    // bonk vars are set in setBonk()

    streamSize = vals.size();
    isThreshed = false;
    thresh = 0.0;
    isDecayingGrowing = false;
    decayGrowRatio = 1.0;
    isNormalized = false;
    valRange = ofVec2f(0,1);
    isClamped = false;
    isBonked = false;
    smoothingType = SMOOTHING_NONE;
    slideUp = 1;
    slideDown = 1;
}
//-------------------------------------------------------------------------
void ofxDataStream::initAccum(int _depth){
    smoothingType = SMOOTHING_ACCUM;
    meanType = MEAN_ARITH;

    // reset if previously initiated
    if (smoothHistos.size() > 0) {
        for (int a=0; a<smoothHistos.size(); a++) {
            smoothHistos.erase(smoothHistos.begin());
        }
    }
    // create new history
    vector<float> tempVector;
    for (int d=0; d<_depth; d++) {
        tempVector.push_back(0);
    }

    histoSize = tempVector.size();

    // store a history for each stream member
    for (int i=0; i<streamSize; i++) {
        smoothHistos.push_back(tempVector);
    }
}

void ofxDataStream::initSlide(float _sU, float _sD){
    if (_sU == 0 || _sD == 0) {
        ofLogError("ofxDataStream") << "slide values must be non-zero";
        return;
    }
    smoothingType = SMOOTHING_SLIDE;
    slideUp = _sU;
    slideDown = _sD;
}
//-------------------------------------------------------------------------
void ofxDataStream::update(const vector<float>& _vals) {
    if (_vals.size() != streamSize) {
        ofLogError("ofxDataStream") << "update(): vector size mismatch";
        return;
    }

    // reset max values
    maxValue = 0.0;
    maxValueN = 0.0;
    
    for (int i=0; i<streamSize; i++) {
        update(_vals[i], i);
        if (vals[i] > maxValue) {
            maxValue = vals[i];
        }
        if (valsN[i] > maxValueN) {
            maxValueN = valsN[i];
        }
    }
}

void ofxDataStream::update(float _val, int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "update(): index doesn't exist";
        return;
    }

    // store the new value
    vals[_idx] = _val;

    // get the delta value
    // (used in the smooth method)
    deltaVals[_idx] = vals[_idx] - prevVals[_idx];

    // smooth vals
    if (smoothingType == SMOOTHING_ACCUM ||
        smoothingType == SMOOTHING_SLIDE) {
        vals[_idx] = smooth(_idx, vals[_idx]);
    }
    
    if (isDecayingGrowing) {
        float valDiff = vals[_idx] - (vals[_idx] * decayGrowRatio);
        vals[_idx] -= valDiff * ofGetLastFrameTime();
    }

    // clamp the value
    if (isClamped) {
        vals[_idx] = ofClamp(vals[_idx], clampLo, clampHi);
    }
    
    // update bonks
    if (isBonked) {
        // recalculate delta for smoothing/clamping
        float smoothedDelta = vals[_idx] - prevVals[_idx];
        
        if (smoothedDelta >= bonkHi) {
            bonkVals[_idx] = true;
        }
        else if (smoothedDelta <= bonkLo) {
            bonkVals[_idx] = false;
        }
        
        if (bonkVals[_idx] && !bonkPrevVals[_idx]) {
            bonks[_idx] = true;
        }
        else bonks[_idx] = false;
        
        bonkPrevVals[_idx] = bonkVals[_idx];
    }

    // store the new value in previous value
    prevVals[_idx] = vals[_idx];

    // set trigger
    if (isThreshed) triggers[_idx] = vals[_idx] > thresh;

    // end of the line for vals
    valsN[_idx] = vals[_idx];

    // normalize the value
    if (isNormalized) {
        valsN[_idx] = ofClamp(valsN[_idx], valRange.x, valRange.y);
        valsN[_idx] = (valsN[_idx] - valRange.x) / (valRange.y - valRange.x);
    }
}

float ofxDataStream::smooth(int _idx, float _val) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "smooth(): index doesn't exist";
        return;
    }

    float smoothedValue = 0.0;

    if (smoothingType == SMOOTHING_ACCUM) {
        smoothHistos[_idx].erase(smoothHistos[_idx].begin());
        smoothHistos[_idx].push_back(_val);

        // get averages
        if (meanType == MEAN_ARITH) {
            for (int i=0; i<histoSize; i++) {
                smoothedValue += smoothHistos[_idx][i];
            }
            smoothedValue /= histoSize;
        }
        else if (meanType == MEAN_GEOM) {
            for (int i=0; i<histoSize; i++) {
                if (i==0) smoothedValue = smoothHistos[_idx][i];
                smoothedValue *= smoothHistos[_idx][i];
            }
            smoothedValue = powf(smoothedValue, 1.0/histoSize);
        }
        else if (meanType == MEAN_HARM) {
            for (int i=0; i<histoSize; i++) {
                if (i==0) smoothedValue = smoothHistos[_idx][i];
                smoothedValue += 1/smoothHistos[_idx][i];
            }
            smoothedValue = histoSize / smoothedValue;
        }
    }
    else if (smoothingType == SMOOTHING_SLIDE) {
        if (deltaVals[_idx] >= 0) smoothedValue = prevVals[_idx] + (deltaVals[_idx]/slideUp);
        else smoothedValue = prevVals[_idx] + (deltaVals[_idx]/slideDown);
    }
    else {ofLogError("ofxDataStream") << "smooth(): not valid smoothing type";}

    return smoothedValue;
}
//-------------------------------------------------------------------------
void ofxDataStream::setThresh(float _t) {
    isThreshed = true;
    thresh = _t;
}

void ofxDataStream::setThreshN(float _tN) {
    isThreshed = true;
    thresh = valRange.x + (_tN * (valRange.y - valRange.x));
}

float ofxDataStream::getThresh() {return thresh;}

float ofxDataStream::getThreshN() {return (thresh - valRange.x) / (valRange.y - valRange.x);}

void ofxDataStream::setDecayGrow(bool _isDG, float _ratio) {
    isDecayingGrowing = _isDG;
    decayGrowRatio = _ratio;
}

bool ofxDataStream::getDecayingGrowing() {return isDecayingGrowing;}

void ofxDataStream::setNormalized(bool _n, ofVec2f _range) {
    if (_range.y - _range.x == 0) {
        ofLogError("ofxDataStream") << "setNormalized(): value range cannot be zero";
        return;
    }
    isNormalized = _n;
    valRange = _range;
}

void ofxDataStream::setRange(ofVec2f _range) {
    valRange = _range;
}
void ofxDataStream::setRangeLo(int _idx) {
    valRange.x = vals[_idx];
}
void ofxDataStream::setRangeHi(int _idx) {
    valRange.y = vals[_idx];
}
ofVec2f ofxDataStream::getRange() {return valRange;}
//-------------------------------------------------------------------------
void ofxDataStream::setClamp(bool _c, float _lo, float _hi) {
    isClamped = _c;
    clampLo = _lo;
    clampHi = _hi;
}

void ofxDataStream::setBonk(float _hiThresh, float _loThresh) {
    isBonked = true;
    bonkHi = _hiThresh;
    bonkLo = _loThresh;
    
    for (int v=0; v<streamSize; v++) {
        bonkVals.push_back(false);
        bonkPrevVals.push_back(false);
        bonks.push_back(false);
    }
}

//-------------------------------------------------------------------------

float ofxDataStream::getValue(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getValue(): index doesn't exist";
        return 0;
    }
    return vals[_idx];
}

float ofxDataStream::getValueN(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getValue(): index doesn't exist";
        return 0;
    }
    return valsN[_idx];
}

float ofxDataStream::getDeltaValue(int _idx) {
    if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getDeltaValue(): index doesn't exist";
        return 0;
    }
    return deltaVals[_idx];
}

bool ofxDataStream::getTrigger(int _idx) {
    bool returnedTrig = false;

    if (_idx >= 0 || _idx < streamSize) {
        returnedTrig = triggers[_idx];
    }
    else ofLogError("ofxDataStream") << "getTrigger(): index doesn't exist";

    return returnedTrig;
}

bool ofxDataStream::getBonk(int _idx) {
    bool returnedBonk = false;
    
    if (!isBonked) {
        ofLogError("ofxDataStream") << "getBonk(): need to call setBonk first";
    }
    else if (_idx < 0 || _idx >= streamSize) {
        ofLogError("ofxDataStream") << "getBonk(): index doesn't exist";
    }
    else returnedBonk = bonks[_idx];
    
    return returnedBonk;
}

float ofxDataStream::getMaxVal() {return maxValue;}

float ofxDataStream::getMaxValN() {return maxValueN;}

void ofxDataStream::setMeanType(ofxDataStream::Mean_t _type) {meanType = _type;}
//-------------------------------------------------------------------------
const vector<float>& ofxDataStream::getStream() {return vals;}
const vector<float>& ofxDataStream::getStreamN() {return valsN;}
const vector<bool>& ofxDataStream::getTriggers() {return triggers;}
const vector<float>& ofxDataStream::getDeltas() {return deltaVals;}
const vector<bool>& ofxDataStream::getBonks() {return bonks;}
