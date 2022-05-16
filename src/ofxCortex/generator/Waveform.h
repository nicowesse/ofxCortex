#pragma once

#include "ofMathConstants.h"
#include "ofMath.h"

namespace ofxCortex { namespace core { namespace generator {

class Waveform {
  
public:
  static double sine(double x, double period, double amplitude, double shift = 0.0)
  {
    return sin(cycle(x, period, shift)) * amplitude;
  }
  
  static double square(double x, double period, double amplitude, double shift = 0.0)
  {
    return ofSign(sin(cycle(x, period, shift))) * amplitude;
  }
  
  static double periodToFreq(double period) { return 1.0 / period; }
  static double freqToPeriod(double freq) { return 1.0 / freq; }
  
private:
  Waveform() = default;
  
  static double cycle(double x, double period, double shift = 0.0)
  {
    double freq = 1.0 / period;
    return 2.0 * PI * (x - fmod(shift, period)) * freq;
  }
  
};

}}}
