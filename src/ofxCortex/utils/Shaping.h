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
  
  static double expBlend(double x, double slope = 0.5, double shift = 0.5)
  {
    double c = 2.0 / (1.0 - slope) - 1.0;
    
    auto f = [&](double x, double n) { return pow(x, c) / pow(n, c - 1.0); };
    
    return (x < shift) ? f(x, shift) : 1.0 - f(1.0 - x, 1.0 - shift);
  };
  
  static float parabola( float x, float k )
  {
      return pow( 4.0*x*(1.0-x), k );
  }
  
  static float signedToUnsigned(float x) { return (x + 1.0) / 2.0; }
  static float unsignedToSigned(float x) { return (x * 2.0) - 1.0; }
};

class ShapingFunction {
public:
  ShapingFunction() {};
  
  virtual double operator()(double x) = 0;
  static double eval(double x) {};
  
  ofParameterGroup parameters;
protected:
};

}}}
