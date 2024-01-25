#pragma once

namespace ofxCortex::core::utils {

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
  Lerped(T value = T(), float smoothing = 0.1f, const std::string & name = "Lerped Value") : current(value), target(value)
  {
    ofAddListener(ofEvents().update, this, &Lerped::update);
    
    parameters.setName(name);
    parameters.add(currentValue, this->smoothing);
    this->smoothing.set(smoothing);
  }
  
  ~Lerped()
  {
    ofRemoveListener(ofEvents().update, this, &Lerped::update);
  }
  
  void operator=(const T& value) { this->setTarget(value); }
  operator T() const { return current; }
  
  void setTarget(const T & value) { this->target = value; }
  void setCurrent(const T & value) { this->current = value; }
  void setSmoothing(float value) { this->smoothing = value; }
  
  
  ofParameterGroup parameters;
protected:
  T current;
  T target;
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> smoothing { "Smoothing", 0.1, 0.0, 1.0 };
  
  void update(ofEventArgs & e) { current = ofLerp(current, target, 1.0 - pow(this->smoothing.get(), ofGetLastFrameTime())); currentValue = ofToString(current); }
};

template<>
inline void Lerped<glm::vec3>::update(ofEventArgs & e)
{
  float t = 1.0 - pow(this->smoothing.get(), ofGetLastFrameTime());
  current = glm::mix(current, target, t);
}

};
