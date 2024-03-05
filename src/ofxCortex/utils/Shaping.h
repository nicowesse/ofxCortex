#pragma once

#include "ofLog.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofParameterGroup.h"

namespace ofxCortex { namespace core { namespace utils {

class ShapingFunction {
public:
  ShapingFunction() {};
  
  virtual double operator()(double x) = 0;
  virtual void draw(const ofRectangle & viewport)
  {
    float inset = 16;
    int steps = viewport.width / 4.0f;
    
    ofPushStyle();
    
    ofFill();
    ofSetColor(12);
    ofDrawRectRounded(viewport, 6);

    ofNoFill();
    ofSetColor(255, 32);
    ofDrawRectRounded(viewport, 6);
    
    ofSetColor(64);
    ofDrawLine(viewport.getLeft() + inset, viewport.getBottom() - inset, viewport.getRight() - inset, viewport.getBottom() - inset);
    ofDrawLine(viewport.getLeft() + inset, viewport.getBottom() - inset, viewport.getLeft() + inset, viewport.getTop() + inset);
    
    ofFill();
    ofDrawCircle(viewport.getLeft() + inset, viewport.getTop() + inset, 2);
    ofDrawCircle(viewport.getRight() - inset, viewport.getBottom() - inset, 2);
    
    ofNoFill();
    ofSetColor(255);
    ofBeginShape();
    for (int i = 0; i <= steps; i++)
    {
      float t = (float) i / steps;
      float x = ofMap(t, 0, 1, viewport.getLeft() + inset, viewport.getRight() - inset, true);
      float y = ofMap((*this)(t), 0, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true);
      
      ofVertex(x, y);
    }
    ofEndShape();
    
    ofFill();
    ofSetColor(128);
    ofDrawCircle(viewport.getLeft() + inset, ofMap((*this)(0.0), 0, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
    ofDrawCircle(viewport.getRight() - inset, ofMap((*this)(1.0), 0, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
    
    ofDrawBitmapString(parameters.getName(), viewport.getLeft() + inset + 8, viewport.getTop() + inset + 11 + 4);
    
    ofPopStyle();
  }
  
  ofParameterGroup parameters;
  operator ofParameterGroup&() { return parameters; }
  
protected:
  
};

class LinearFunction : public ShapingFunction {
public:
  LinearFunction(const std::string & name = "Linear Function") {
    parameters.setName(name);
  };
  
  virtual double operator()(double x) override { return x; }
};

class InvertFunction : public ShapingFunction {
public:
  InvertFunction(const std::string & name = "Invert Function") {
    parameters.setName(name);
  };
  
  virtual double operator()(double x) override { return 1.0 - x; }
};

class GainFunction : public ShapingFunction {
public:
  GainFunction(const std::string & name = "Gain Function") {
    parameters.setName(name);
    parameters.add(k_p);
  };
  
  virtual double operator()(double x) override { return (*this)(x, k_p.get()); }
  double operator()(double x, double k)
  {
    const float a = 0.5 * pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
    return (x < 0.5) ? a : 1.0 - a;
  }
  
protected:
  ofParameter<float> k_p { "K", 0.5, 0.0, 1.0 };
  
};

class BiasedGainFunction : public ShapingFunction {
public:
  BiasedGainFunction(const std::string & name = "Biased Gain Function") {
    parameters.setName(name);
    parameters.add(amplitude, bias);
  };
  
  virtual double operator()(double x) override { return (*this)(x, amplitude.get(), bias.get()); }
  virtual double operator()(double x, double a, double b) { return pow(x, a) / (pow(x, a) + pow(b - b * x, a)); }
  
protected:
  ofParameter<float> amplitude { "Amplitude", 2.0, 0.0, 4.0 };
  ofParameter<float> bias { "Bias", 0.5, 0.0, 1.0 };
  
};

class ParabolaFunction : public ShapingFunction {
public:
  ParabolaFunction(const std::string & name = "Parabola Function") {
    parameters.setName(name);
    parameters.add(k_p);
  };
  
  virtual double operator()(double x) override { return (*this)(x, k_p.get()); }
  double operator()(double x, double k) { return pow(4.0 * x * (1.0 - x), k); }
  
protected:
  ofParameter<float> k_p { "K", 1.0, 0.0, 6.0 };
  
};

class ExpBlendFunction : public ShapingFunction {
public:
  ExpBlendFunction(const std::string & name = "Exponential Blend Function") {
    parameters.setName(name);
    parameters.add(slope, shift);
  };
  
  virtual double operator()(double x) override
  {
    return (*this)(x, slope.get(), shift.get());
  }
  
  virtual double operator()(double x, double slope, double shift)
  {
    double c = 2.0 / (1.0 - slope) - 1.0;
    
    auto f = [&](double x, double n) { return pow(x, c) / pow(n, c - 1.0); };
    
    return (x < shift) ? f(x, shift) : 1.0 - f(1.0 - x, 1.0 - shift);
  }
  
protected:
  ofParameter<float> slope { "Slope", 0.5, -0.999, 0.999 };
  ofParameter<float> shift { "Shift", 0.5, 0.0, 1.0 };
  
};

class ExpStepFunction : public ShapingFunction {
public:
  ExpStepFunction() {
    parameters.setName("Exponential Step Function");
    parameters.add(k_p, n_p);
  };
  
  virtual double operator()(double x) override { return (*this)(x, k_p.get(), n_p.get()); }
  
  virtual double operator()(double x, double k, double n) { return exp(-k * pow(x, n)); }
  
protected:
  ofParameter<float> k_p { "K", 1.0, 0.0, 4.0 };
  ofParameter<float> n_p { "N", 0.5, 0.0, 1.0 };
  
};

class Shaping {
public:
  inline static LinearFunction linear;
  inline static InvertFunction invert;
  inline static GainFunction gain;
  inline static BiasedGainFunction biasedGain;
  inline static ExpBlendFunction expBlend;
  inline static ExpStepFunction expStep;
  inline static ParabolaFunction parabola;
  
  static float signedToUnsigned(float x) { return (x + 1.0) / 2.0; }
  static float unsignedToSigned(float x) { return (x * 2.0) - 1.0; }
  
  template<typename T>
  static T interpolate(const std::vector<T> & values, float t)
  {
    if (values.size() == 1 || ofIsFloatEqual(t, 0.0f)) return values[0];
    if (ofIsFloatEqual(t, 1.0f)) return values[values.size() - 1];
    
    float valueT = t * (values.size() - 1);
    int lower = floor(valueT);
    int upper = ceil(valueT);
    float interpolateT = ofClamp(valueT - lower, 0.0, 1.0);
    
    return ofInterpolateCosine(values[lower], values[upper], interpolateT);
  }
  
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

}}}
