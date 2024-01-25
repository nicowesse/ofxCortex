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

};
