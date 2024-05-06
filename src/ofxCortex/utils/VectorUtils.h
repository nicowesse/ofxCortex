#pragma once

#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

#include "ofMath.h"
#include "ofVectorMath.h"

namespace ofxCortex { namespace core { namespace utils {

namespace Vector {

inline static glm::vec2 random2D(float radius = 1.0) { return glm::circularRand(MAX(radius, std::numeric_limits<float>::epsilon())); }
inline static glm::vec3 random3D(float radius = 1.0) { return glm::sphericalRand(MAX(radius, std::numeric_limits<float>::epsilon())); }

inline static glm::vec3 to3D(const glm::vec2 & v, float z = 0.0f) { return { v.x, v.y, z }; }
static std::vector<glm::vec3> to3D(const std::vector<glm::vec2> & vec, float z = 0.0f)
{
  std::vector<glm::vec3> output;
  for (const glm::vec2 & p : vec) output.push_back({ p.x, p.y, z });
  return output;
}

template<typename T>
inline static void translate(std::vector<T> & v, const T & p) { std::for_each(v.begin(), v.end(), [&p](T & e) { e += p; }); }

inline static glm::vec2 fromRadians(float angle) { return { cosf(angle), sinf(angle) }; }
inline static glm::vec2 fromDegrees(float angle) { return fromRadians(ofDegToRad(angle)); }
inline static glm::vec2 fromAngle(float angle) { return fromDegrees(angle); }

inline static float toRadians(const glm::vec2 & v) { return glm::atan(v.y, v.x); }
inline static float toDegrees(const glm::vec2 & v) { return ofRadToDeg(toRadians(v)); }
inline static float toAngle(const glm::vec2 & v) { return toDegrees(v); }

static float angleBetween(const glm::vec2 & a, const glm::vec2 & b) { return std::atan2(b.y, b.x) - std::atan2(a.y, a.x); }
static float angleBetween(const glm::vec3 & a, const glm::vec3 & b)
{
  const glm::vec3 u = glm::cross(a, b);
  return atan2(glm::length(u), glm::dot(a, b)) * ofSign(u.z || 1);
}

template<typename T> inline static void flipX(T & v) { v.x * -1.0f; }
template<typename T> inline static void flipY(T & v) { v.y * -1.0f; }

template<typename T> inline static T getFlippedY(const T & v) { return v * T(1, -1, 1); }

static glm::vec2 getRotatedAroundOrigin(const glm::vec2 & point, const glm::vec2 & origin, float radians)
{
  const float co = cos(radians);
  const float si = sin(radians);
  
  float nx = (co * (point.x - origin.x)) + (si * (point.y - origin.y)) + origin.x;
  float ny = (co * (point.y - origin.y)) - (si * (point.x - origin.x)) + origin.y;
  return { nx, ny };
}

inline static void rotateAroundOrigin(glm::vec2 & point, const glm::vec2 & origin, float radians) { point = getRotatedAroundOrigin(point, origin, radians); }

inline static glm::vec2 getRotated(const glm::vec2 & v, float radians)
{
  return glm::vec2(v.x * cos(radians) - v.y * sin(radians), v.x * sin(radians) + v.y * cos(radians));
}

template<typename T> inline static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }
template<typename T> inline static T getLimited(const T & v, float max) { return glm::min(v, glm::normalize(v) * max); }

}


}}}
