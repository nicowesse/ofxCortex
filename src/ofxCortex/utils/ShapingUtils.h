#pragma once

#include "ofLog.h"
#include "ofUtils.h"
#include "ofGraphics.h"
#include "ofMath.h"
#include "ofParameterGroup.h"
#include "ofJson.h"
#include "ofxCortex/types/Plot.h"
//#include "ofxCortex/core/Serialization.h"

namespace ofxCortex { namespace core { namespace utils {

class ShapingUtils {
public:
  static void drawFunction(const ofRectangle & viewport, const std::string & label, const std::function<double(double)> & func, float resolution = 2.0)
  {
    drawFunctions(viewport, std::vector<std::string>{ label }, [&](double x) -> std::vector<double> { return { func(x) }; });
  }
  
  static void drawFunctions(const ofRectangle & viewport, const std::vector<std::string> & labels, const std::function<std::vector<double>(double)> & func, float resolution = 2.0, float min = 0.0, float max = 1.0)
  {
    static std::vector<ofColor> palette = { ofColor::white, ofColor::tomato, ofColor::springGreen, ofColor::magenta, ofColor::cyan, ofColor::lime, ofColor::aqua };
    const float inset = 16;
    
    int steps = viewport.width / resolution;
    
    size_t functions = func(0.0).size();
  
    std::map<uint8_t, std::vector<float>> evaluated;
    for (int step = 0; step <= steps; step++)
    {
      auto results = func((float) step / steps);
      for (int i = 0; i < results.size(); i++) evaluated[i].push_back(results[i]);
    }
    
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
    
    for (auto & [functionIndex, values] : evaluated)
    {
      const ofColor & c = palette[functionIndex % palette.size()];
      ofNoFill();
      ofSetColor(c, functionIndex == 0 ? 255 : 112);
      ofBeginShape();
      for (int i = 0; i < values.size(); i++)
      {
        float t = (float) i / values.size();
        float x = ofMap(t, 0, 1, viewport.getLeft() + inset, viewport.getRight() - inset, true);
        float y = ofMap(values[i], min, max, viewport.getBottom() - inset, viewport.getTop() + inset, true);
        
        ofVertex(x, y);
      }
      ofEndShape();
      
      ofFill();
      ofSetColor(palette[functionIndex % palette.size()], 192);
      ofDrawCircle(viewport.getLeft() + inset, ofMap(values.front(), min, max, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
      ofDrawCircle(viewport.getRight() - inset, ofMap(values.back(), min, max, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
      
      ofSetColor(c, 128);
      ofDrawBitmapString(labels[functionIndex], viewport.getLeft() + inset + 12, viewport.getTop() + inset + 11 + 4 + (functionIndex * 14));
    }
    
    
    
    ofPopStyle();
  }
};

class ShapingFunctionAbstract {
public:
  explicit ShapingFunctionAbstract(const std::string & name = "Shaping Function") : parameters(name) {
    ofParameter<ofxCortex::core::types::Plot> plotter { "Plot", ofxCortex::core::types::Plot([this](float x){ return (*this)((double)x); }) };
    parameters.add(plotter);
  }
  
  // Virtual destructor (essential for abstract classes)
  virtual ~ShapingFunctionAbstract() = default;
  
  // Copy operations - defaulted but declared
  ShapingFunctionAbstract(const ShapingFunctionAbstract&) = default;
  ShapingFunctionAbstract& operator=(const ShapingFunctionAbstract&) = default;

  // Move operations
  ShapingFunctionAbstract(ShapingFunctionAbstract&&) noexcept = default;
  ShapingFunctionAbstract& operator=(ShapingFunctionAbstract&&) noexcept = default;
  
  
  [[nodiscard]] virtual double operator()(double x) const = 0;
  
  virtual double min() const { return 0.0; }
  virtual double max() const { return 1.0; }
  
  
  virtual void draw(const ofRectangle & viewport)
  {
    const auto& name = parameters.getName();
    ShapingUtils::drawFunction(viewport, name, [this](double x) { return this->operator()(x); });
  }
  
  ofParameterGroup parameters;
  operator ofParameterGroup&() { return parameters; }
  
  template<typename T>
  T getParameterValue(const std::string & name) const { return parameters.get<T>(name).get(); }
  
  virtual ofJson toJSON() { return ofJson(); }
  virtual void fromJSON(const ofJson & json) {}
  
  friend std::ostream& operator<<(std::ostream& os, ShapingFunctionAbstract& func) { return os; }
  friend std::istream& operator>> (std::istream &is, ShapingFunctionAbstract &func) { return is; }
};



class LinearFunction : public ShapingFunctionAbstract {
public:
  LinearFunction(const std::string & name = "Linear Function") {
    parameters.setName(name);
  };
  
  virtual double operator()(double x) const override { return x; }
};

class InvertFunction : public ShapingFunctionAbstract {
public:
  InvertFunction(const std::string & name = "Invert Function") {
    parameters.setName(name);
  };
  
  virtual double operator()(double x) const override { return 1.0 - x; }
};

class GainFunction : public ShapingFunctionAbstract {
public:
  GainFunction(const std::string & name = "Gain Function") {
    parameters.setName(name);
    parameters.add(k_p);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, k_p.get()); }
  double operator()(double x, double k) const
  {
    const float a = 0.5 * pow(2.0 * ((x < 0.5) ? x : 1.0 - x), k);
    return (x < 0.5) ? a : 1.0 - a;
  }
  
protected:
  ofParameter<float> k_p { "Contrast", 0.5, 0.0, 8.0 };
  
};

class PulseWidthFunction : public ShapingFunctionAbstract {
public:
  PulseWidthFunction() {
    parameters.setName("Pulse Width Function");
    parameters.add(start, end);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, start.get(), end.get()); }
  virtual double operator()(double x, double start, double end) const {
    return (x >= start && x <= end);
  }
  
protected:
  ofParameter<float> start  { "Start",  0.0, 0.0, 1.0 };
  ofParameter<float> end { "End", 1.0, 0.0, 1.0 };
  
};

class ExponentialFunction : public ShapingFunctionAbstract {
public:
  ExponentialFunction(const std::string & name = "Exponential Function") {
    parameters.setName(name);
    parameters.add(exponent);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, exponent.get()); }
  double operator()(double x, double exponent) const { return pow(x, exponent); }
  
protected:
  ofParameter<float> exponent { "Exponent", 2.0, 0.0, 8.0 };
  
};

class BiasedGainFunction : public ShapingFunctionAbstract {
public:
  BiasedGainFunction(const std::string & name = "Biased Gain Function") {
    parameters.setName(name);
    parameters.add(amplitude, bias);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, amplitude.get(), bias.get()); }
  virtual double operator()(double x, double a, double b) const { return pow(x, a) / (pow(x, a) + pow(b - b * x, a)); }
  
protected:
  ofParameter<float> amplitude { "Amplitude", 2.0, 0.0, 4.0 };
  ofParameter<float> bias { "Bias", 0.5, 0.0, 1.0 };
  
};

class ParabolaFunction : public ShapingFunctionAbstract {
public:
  ParabolaFunction(const std::string & name = "Parabola Function") {
    parameters.setName(name);
    parameters.add(k_p);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, k_p.get()); }
  double operator()(double x, double k) const { return pow(4.0 * x * (1.0 - x), std::max(0.001, k)); }
  
protected:
  ofParameter<float> k_p { "Shape", 1.0, 0.0001, 6.0 };
  
};

class ExpBlendFunction : public ShapingFunctionAbstract {
public:
  ExpBlendFunction(const std::string & name = "Exponential Blend Function") {
    parameters.setName(name);
    parameters.add(slope, shift);
  };
  
  virtual double operator()(double x) const override
  {
    return (*this)(x, slope.get(), shift.get());
  }
  
  virtual double operator()(double x, double slope, double shift) const
  {
    slope = ofClamp(slope, -0.99, 0.99);
    double c = 2.0 / (1.0 - slope) - 1.0;
    
    auto f = [&](double x, double n) { return pow(x, c) / pow(n, c - 1.0); };
    
    return (x < shift) ? f(x, shift) : 1.0 - f(1.0 - x, 1.0 - shift);
  }
  
protected:
  ofParameter<float> slope { "Slope", 0.5, -0.999, 0.999 };
  ofParameter<float> shift { "Shift", 0.5, 0.0, 1.0 };
  
};

class ExpStepFunction : public ShapingFunctionAbstract {
public:
  ExpStepFunction(const std::string & name = "Exponential Step Function") {
    parameters.setName(name);
    parameters.add(k_p, n_p);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, k_p.get(), n_p.get()); }
  virtual double operator()(double x, double k, double n) const { return exp(-k * pow(x, n)); }
  
protected:
  ofParameter<float> k_p { "K", 1.0, 0.0, 4.0 };
  ofParameter<float> n_p { "N", 0.5, 0.0, 1.0 };
  
};

class SmoothstepFunction : public ShapingFunctionAbstract {
public:
  SmoothstepFunction(const std::string & name = "Smoothstep Function") {
    parameters.setName(name);
    parameters.add(left_edge, right_edge);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, left_edge.get(), right_edge.get()); }
  virtual double operator()(double x, double left_edge, double right_edge) const {
    x = ofClamp((x - left_edge) / (right_edge - left_edge), 0, 1);
    return x*x*x/(3.0*x*x-3.0*x+1.0);
  }
  
protected:
  ofParameter<float> left_edge  { "Left Edge",  0.0, 0.0, 1.0 };
  ofParameter<float> right_edge { "Right Edge", 1.0, 0.0, 1.0 };
  
};

class SineFunction : public ShapingFunctionAbstract {
public:
  SineFunction(const std::string & name = "Sine Function") {
    parameters.setName(name);
    parameters.add(frequency, offset);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, frequency.get(), offset.get()); }
  virtual double operator()(double x, double f, double o) const { return sin(x * TWO_PI * f + offset * TWO_PI) * 0.5 + 0.5; }
  
protected:
  ofParameter<float> frequency  { "Frequency (hz)",  1.0, 0.0, 10.0 };
  ofParameter<float> offset { "Offset", 0.0, 0.0, 1.0 };
  
};

class SignedSineFunction : public ShapingFunctionAbstract {
public:
  SignedSineFunction(const std::string & name = "Signed Sine Function") {
    parameters.setName(name);
    parameters.add(frequency, offset);
  };
  
  virtual double min() const override { return -1.0; }
  
  virtual double operator()(double x) const override { return (*this)(x, frequency.get(), offset.get()); }
  virtual double operator()(double x, double f, double o) const { return sin(x * TWO_PI * f + offset); }

protected:
  ofParameter<float> frequency  { "Frequency (hz)",  1.0, 0.0, 10.0 };
  ofParameter<float> offset { "Offset", 0.0, 0.0, 1.0 };
};

class PeriodicSineFunction : public ShapingFunctionAbstract {
public:
  PeriodicSineFunction(const std::string & name = "Signed Sine Function", float maxPeriod = 10.0) {
    parameters.setName("Periodic Sine Function");
    parameters.add(period, offset);
    period.setMax(maxPeriod);
  };
  
  virtual double operator()(double x) const override { return (*this)(x, period.get(), offset.get()); }
  virtual double operator()(double x, double p, double o) const { return sin(x * TWO_PI * (1.0 / p) + offset) * 0.5 + 0.5; }
  
protected:
  ofParameter<float> period  { "Period",  1.0, 0.0, 10.0 };
  ofParameter<float> offset { "Offset", 0.0, 0.0, 1.0 };
  
};

class TriangleFunction : public ShapingFunctionAbstract {
public:
  TriangleFunction(const std::string & name = "Triangle Function") {
    parameters.setName(name);
  };
  
  virtual double operator()(double x) const override { return 2.0 * std::min(x, 1.0 - x); }
};

class Shaping {
public:
  inline static LinearFunction linear;
  inline static InvertFunction invert;
  inline static PulseWidthFunction pulseWidth;
  inline static TriangleFunction triangle;
  inline static GainFunction gain;
  inline static BiasedGainFunction biasedGain;
  inline static ExpBlendFunction expBlend;
  inline static ExpStepFunction expStep;
  inline static ParabolaFunction parabola;
  inline static SmoothstepFunction smoothstep;
  inline static SineFunction sine;
  inline static SignedSineFunction signedSine;
  inline static PeriodicSineFunction periodicSine;
  
  static float signedToUnsigned(float x) { return (x + 1.0) / 2.0; }
  static float unsignedToSigned(float x) { return (x * 2.0) - 1.0; }
};

}}}
