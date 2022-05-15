#pragma once

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "of3dGraphics.h"

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

static glm::vec2 randomInRectangle(const ofRectangle & rect)
{
  return { (int) ofRandom(rect.getLeft(), rect.getRight()), (int) ofRandom(rect.getTop(), rect.getBottom()) };
}

static bool isInsideBox(const ofBoxPrimitive & box, const glm::vec3 & p)
{
  return (p.x >= 0.0f && p.x <= box.getWidth()) && (p.y >= 0.0f && p.y <= box.getHeight()) && (p.z >= 0.0f && p.z <= box.getDepth());
}

namespace Vector {
static glm::vec2 random2D(float radius = 1.0)
{
  float a = ofRandom(TWO_PI);
  return { cos(a) * radius, sin(a) * radius };
}

static glm::vec3 random3D(float radius = 1.0)
{
  float phi = ofRandom(0, TWO_PI);
  float costheta = ofRandom(-1,1);

  float theta = glm::acos(costheta);
  return glm::vec3{ sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta) } * radius;
}

static glm::vec3 to3D(const glm::vec2 & v, float z = 0.0f) { return { v.x, v.y, z }; }
static std::vector<glm::vec3> to3D(const std::vector<glm::vec2> & vec, float z = 0.0f)
{
  std::vector<glm::vec3> output;
  for (const glm::vec2 & p : vec) output.push_back({ p.x, p.y, z });
  return output;
}

static glm::vec2 fromAngle(float angle) { return { cosf(angle), sinf(angle) }; }
static float toAngle(const glm::vec2 & v) { return glm::atan(v.y, v.x); }

//static void limit(glm::vec2 & v, float max) { if (glm::length2(v) > max * max) v = glm::normalize(v) * max; }
template<typename T>
static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }

}

}}
