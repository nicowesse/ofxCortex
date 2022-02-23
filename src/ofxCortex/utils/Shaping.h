#pragma once

namespace ofxCortex { namespace utils {

class Shaping {
public:
  
  static float gain(float x, float k)
  {
    const float a = 0.5 * pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
    return (x < 0.5) ? a : 1.0 - a;
  }
  
  static float parabola( float x, float k )
  {
      return pow( 4.0*x*(1.0-x), k );
  }
};

}}
