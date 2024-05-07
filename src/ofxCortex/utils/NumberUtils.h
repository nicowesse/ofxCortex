#pragma once

#include "ofAppRunner.h"
#include "ofParameter.h"

namespace ofxCortex { namespace core { namespace utils {

static float roundToNearest(float value, float multiple)
{
  if (multiple == 0)
    return value;
  
  int remainder = fmod(abs(value), multiple);
  if (remainder == 0)
    return value;
  
  if (value < 0)
    return -(abs(value) - remainder);
  else
    return value + multiple - remainder;
};

static double floorToNearest(double value, double multiple)
{
  double divider = 1.0 / multiple;
  return floor(value * divider) / divider;
}

static unsigned long modulo(int a, int b) { return (b + (a % b)) % b; }

template<typename T>
static double normalizeIndex(size_t index, const std::vector<T> & v) { return (double) index / (v.size() - 1); }

template<typename T = float>
class Lerped {
public:
  Lerped(T value = T(), float _smoothing = 0.1f, const std::string & name = "Lerped Value") : current(value), target(value)
  {
    ofAddListener(ofEvents().update, this, &Lerped::update);
    
    parameters.setName(name);
    parameters.add(currentValue, smoothing);
    this->smoothing.set(_smoothing);
  }
  
  ~Lerped()
  {
    ofRemoveListener(ofEvents().update, this, &Lerped::update);
  }
  
  void operator=(const T& value) { this->setTarget(value); }
  operator T() const { return current; }
  operator ofParameterGroup&() { return parameters; }
  
  void setTarget(const T & value) { this->target = value; }
  void setCurrent(const T & value) { this->current = value; }
  void setSmoothing(float value) { this->smoothing = value; }
  
  
  ofParameterGroup parameters;
protected:
  T current;
  T target;
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> smoothing { "Smoothing", 0.1, 0.0, 1.0 };
  
  void update(ofEventArgs & e) {
    if (ofIsFloatEqual(current, target)) { current = target; return; }
    
    current = ofLerp(current, target, 1.0 - exp(-smoothing.get() * ofGetLastFrameTime()));
    currentValue = ofToString(current);
  }
};

template<>
inline void Lerped<glm::vec3>::update(ofEventArgs & e)
{
  if (current == target) return;
  
  current = glm::mix(current, target, 1.0 - exp(-smoothing.get() * ofGetLastFrameTime()));
}



template<typename T>
class InertialLerp {
public:
  InertialLerp(T value = T(), const std::string & name = "Lerped Value") : current(value), previousTarget(value), target(value), velocity(0)
  {
    ofAddListener(ofEvents().update, this, &InertialLerp::update);
    
    parameters.setName(name);
    parameters.add(currentValue, stiffness, damping, anticipation);
  }
  
  ~InertialLerp()
  {
    ofRemoveListener(ofEvents().update, this, &InertialLerp::update);
  }
  
  void operator=(const T& value) { this->setTarget(value); }
  operator T() const { return current; }
  operator ofParameterGroup&() { return parameters; }
  
  void setTarget(const T & value) { this->target = value; }
  void setCurrent(const T & value) { this->current = value; }
  
  InertialLerp<T> & setStiffness(float value) { this->stiffness = value; return *this; }
  InertialLerp<T> & setDamping(float value) { this->damping = value; return *this; }
  InertialLerp<T> & setAnticipation(float value) { this->anticipation = value; return *this; }
  
  
  ofParameterGroup parameters;
protected:
  T target; // x
  T previousTarget; // xp
  
  T current; // y
  T velocity; // yd
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> stiffness { "Stiffness", 4.0, 0.0, 10.0 }; // f
  ofParameter<float> damping { "Damping", 0.25, 0.0, 1.0 }; // zeta
  ofParameter<float> anticipation { "Anticipation", 0., -2.0, 2.0 }; // r
  
  bool isEqual() const { return current == target; }
  
  void update(ofEventArgs & e) {
    if (isEqual()) return;
    
    const float & f = stiffness.get();
    const float & z = damping.get();
    const float & r = anticipation.get();
    
    float k1 = z / (PI * f);
    float k2 = 1.0 / ((TWO_PI * f) * (TWO_PI * f));
    float k3 = r * z / (TWO_PI * f);
    
    float delta = ofGetLastFrameTime();
    T xd = (target - previousTarget) / delta;
    previousTarget = target;
    
    float k2_stable = std::max(k2, 1.1f * (delta * delta / 4.0f + delta * k1 / 2.0f));
    
    current = current + delta * velocity;
    velocity = velocity + delta * (target + k3 * xd - current - k1 * velocity) / k2_stable;
    
    currentValue = ofToString(current);
  }
};

//template<> bool InertialLerp<float>::isEqual() const { return ofIsFloatEqual(current, target); }

}}}
