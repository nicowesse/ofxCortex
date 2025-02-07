#pragma once

namespace ofxCortex { namespace core { namespace types {

class Plot
{
protected:
  std::function<float(float)> plotFunction;
  float min { 0.0 };
  float max { 0.0 };
  
public:
  Plot() : plotFunction([](float x){ return x; }) {}
  Plot(std::function<float(float)> func, float _min = 0.0, float _max = 1.0) : plotFunction(func), min(_min), max(_max) {};
  Plot(const Plot &other) : plotFunction(other.plotFunction), min(other.min), max(other.max) {}
  
  Plot& operator= (const Plot &other)
  {
    if (this != &other) {
      this->plotFunction = other.plotFunction;
      this->min = other.min;
      this->max = other.max;
    }
    return *this;
  }
  
  void setFunction(std::function<float(float)> func) { this->plotFunction = func; }
  float getMin() const { return min; }
  float getMax() const { return max; }
  
  float operator()(float input) const { return plotFunction(input); }
  
  // Serialization
  friend std::ostream& operator<<(std::ostream& os, const Plot& spacer) { return os; }
  
  // Deserialization
  friend std::istream& operator>> (std::istream &is, Plot &spacer) { return is; }
};
  
}}}
