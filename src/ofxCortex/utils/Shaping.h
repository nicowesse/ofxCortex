#pragma once

namespace ofxCortex { namespace core { namespace utils {

class Shaping {
public:
  
  static float gain(float x, float k)
  {
    const float a = 0.5 * pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
    return (x < 0.5) ? a : 1.0 - a;
  }
  
  static float biasedGain(float x, float amplitude = 2.0f, float bias = 0.5f)
  {
    if (x < 0.0f || x > 1.0f)
    {
      ofLogVerbose("Shaping::gain") << "\n\tThe x value needs to be between 0.0 and 1.0. Returning " << x;
      return x;
    }
    
    //bias = pow((1.0 - bias) + 0.5, 6);
    return pow(x, amplitude) / (pow(x, amplitude) + pow(bias - bias * x, amplitude));
  };
  
  static float parabola( float x, float k )
  {
      return pow( 4.0*x*(1.0-x), k );
  }
};

}}}
