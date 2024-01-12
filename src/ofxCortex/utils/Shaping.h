#pragma once

#include "ofLog.h"
#include "ofUtils.h"
#include "ofParameterGroup.h"

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
  
  template<typename T>
  static T interpolate(const vector<T> & values, float t)
  {
    if (values.size() == 1) return values[0];
    
    std::vector<T> combined;
    for_each_pair(begin(values), end(values), [&](const T & a, const T & b) {
      combined.push_back(ofInterpolateCosine(a, b, t));
    });
    
    if (combined.size() == 1) return combined[0];
    else return interpolate(combined, t);
  }
  
//  template<>
//  static ofColor interpolate(const vector<ofColor> & values, float t)
//  {
//    if (values.size() == 1) return values[0];
//    
//    std::vector<ofColor> combined;
//    for_each_pair(begin(values), end(values), [&](const ofColor & a, const ofColor & b) {
//      combined.push_back(a.getLerped(b, t));
//    });
//    
//    if (combined.size() == 1) return combined[0];
//    else return interpolate(combined, t);
//  }
  
protected:
  template< typename FwdIter, typename Func >
  static Func for_each_pair( FwdIter iterStart, FwdIter iterEnd, Func func )
  {
    if( iterStart == iterEnd )
      return func;
    
    FwdIter iterNext = iterStart;
    ++iterNext;
    
    for( ; iterNext != iterEnd; ++iterStart, ++iterNext )
    {
      func( *iterStart, *iterNext );
    }
    return func;
  }
};

class ShapingFunction {
public:
  ShapingFunction() {};
  
  virtual double operator()(double x) = 0;
  
  ofParameterGroup parameters;
protected:
  
};

class Gain : public ShapingFunction {
public:
  Gain() {
    parameters.setName("Gain");
    parameters.add(k);
  };
  
  virtual double operator()(double x) override
  {
    const float a = 0.5 * pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k.get());
    return (x < 0.5) ? a : 1.0 - a;
  }
  
  ofParameterGroup parameters;
  
protected:
  ofParameter<float> k { "K", 0.5, 0.0, 1.0 };
  
};

class BiasedGain : public ShapingFunction {
public:
  BiasedGain() {
    parameters.setName("Biased Gain");
    parameters.add(amplitude, bias);
  };
  
  virtual double operator()(double x) override
  {
    x = ofClamp(x, 0, 1);
    return pow(x, amplitude.get()) / (pow(x, amplitude.get()) + pow(bias.get() - bias.get() * x, amplitude.get()));
  }
  
  ofParameterGroup parameters;
  
protected:
  ofParameter<float> amplitude { "Amplitude", 2.0, 0.0, 4.0 };
  ofParameter<float> bias { "Bias", 0.5, 0.0, 1.0 };
  
};

}}}
