#pragma once

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"

namespace ofxCortex { namespace utils {

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

template<typename T>
static T randomInVector(std::vector<T> const &v)
{
    auto it = v.cbegin();
    int random = rand() % v.size();
    std::advance(it, random);
 
    return *it;
}

namespace Vector {
static glm::vec2 random2D(float radius = 1.0)
{
  float a = ofRandom(TWO_PI);
  return { cos(a) * radius, sin(a) * radius };
}

static glm::vec2 fromAngle(float angle) { return { cosf(angle), sinf(angle) }; }
static float toAngle(const glm::vec2 & v) { return glm::atan(v.y, v.x); }

//static void limit(glm::vec2 & v, float max) { if (glm::length2(v) > max * max) v = glm::normalize(v) * max; }
static void limit(glm::vec2 & v, float max) { v = glm::min(v, glm::normalize(v) * max); }

}

}}
