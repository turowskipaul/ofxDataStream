# ofxDataStream
A utility addon for OpenFrameworks that manages and smooths streams of data

Copyright (C) 2015 [Paul Turowski] (http://paulturowski.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

This utility class was developed as part of the <i>Being In Time</i> project under the direction of Judith Shatin at the University of Virginia.  This project was funded by an Arts in Action grant from the University of Virginia. More info: [http://judithshatin.com/being-in-time/] (http://judithshatin.com/being-in-time/)

Description
-----------
This addon is designed to simplify the management of various groups of data that each have an arbitrary number of streams.  For example, it might be used with streams of incoming audio data, such as spectral bands, or control data, like Kinect skeleton data.  The class supports different types of smoothing, clamping, normalization, and threshold detection.

Installation
------------
Using ofxDataStream requires [OpenFrameworks] (http://www.openframeworks.cc/) to be installed. To use ofxDataStream, simply copy the class to your OpenFrameworks addons folder, or cd to your addons folder and use the following command:
<pre>git clone https://github.com/turowskipaul/ofxDataStream.git</pre>

Basic Usage Examples
--------------------
declaring instances:
<pre>
ofxDataStream amp;
vector&lt;ofxDataStream&gt; specAmps;
</pre>

initializing slide smoothing:
<pre>
amp.initSlide(60, 20); // slide up, slide down
</pre>

initializing accumulator smoothing:
<pre>
amp.initAccum(100); // size of history
</pre>

initializing clamping and/or normalization
<pre>
amp.setClamp(0.2, 0.8); // clamp min and max
amp.setNormalized(true, 0.2, 0.8); // normalization, min, max
</pre>

set a threshold:
<pre>
amp.setThresh(0.5);
</pre>

get a threshold crossing:
<pre>
amp.getTrigger(0); // index (optional)
</pre>

update with incoming values:
<pre>
amp.update(rawAmplitude, 0); // raw value, index (optional)
specAmps.update(spectralAmplitudes); // or update with a const vector ref
</pre>

get a smoothed value (update updating):
<pre>
amp.getValue(0); // index (optional)
amp.getValueN(0); // get normalized value (index optional)
</pre>

detect "bonks" (onsets):
<pre>
amp.setBonk(0.1, 0.1);  // min growth for onset, min decay
</pre>

set growth/decay:
<pre>
amp.setDecayGrow(true, 0.99); // a framerate-dependent steady decay/growth
</pre>

get direction change time (sec) and depth:
<pre>
// if the direction has changed and
// if the time of change is greater than 0.5 sec
// print the time between changes and amount of change

if (inputStreams[0].getDirectionTimeDiff() > 0.5 &&
        inputStreams[0].directionHasChanged()) {
        ofLogNotice() <<
        "inputStream1 " << inputStreams[0].getDirectionTimeDiff() <<
        " " << inputStreams[0].getDirectionValDiff();
    }
</pre>

Development
-----------
Bug reports, suggestions, and further development are all welcome.
