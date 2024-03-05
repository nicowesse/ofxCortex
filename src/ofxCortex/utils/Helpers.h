#pragma once

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "of3dGraphics.h"
#include "of3dUtils.h"
#include "ofGraphics.h"
#include "ofLog.h"
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>
#include "ofxCortex/types/Box.h"
#include "ofxCortex/utils/Numbers.h"
#include "ofxCortex/utils/Containers.h"
#include "ofxCortex/utils/Shaping.h"
#include <type_traits>


namespace ofxCortex { namespace core { namespace utils {

static glm::vec2 randomInRectangle(const ofRectangle & rect)
{
  return { (int) ofRandom(rect.getLeft(), rect.getRight()), (int) ofRandom(rect.getTop(), rect.getBottom()) };
}

static bool isInsideBox(const ofBoxPrimitive & box, const glm::vec3 & p)
{
  return (p.x >= 0.0f && p.x <= box.getWidth()) && (p.y >= 0.0f && p.y <= box.getHeight()) && (p.z >= 0.0f && p.z <= box.getDepth());
}

static bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R);

static bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh);

static std::vector<glm::vec3> pointsInsideMesh(const std::vector<glm::vec3> & points, const ofMesh & mesh)
{
  std::vector<glm::vec3> output;
  std::copy_if(points.begin(), points.end(), back_inserter(output), [&mesh](const glm::vec3 & p) { return isInsideMesh(p, mesh); });
  return output;
}

static ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh);

static ofRectangle getImageRectangle(const ofBaseDraws & image) { return ofRectangle(0, 0, image.getWidth(), image.getHeight()); }

static ofRectangle getBitmapStringBoundingBox(const std::string & text);

static void drawTexCoordRectangle(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f);

static void drawTexCoordRectangle(const ofRectangle & rect) { drawTexCoordRectangle(rect.x, rect.y, rect.width, rect.height); }

static float getNormalizedTime(float hours = ofGetHours(), float minutes = ofGetMinutes(), float seconds = ofGetSeconds())
{
  float hoursToSeconds = hours * 3600;
  float minutesToSeconds = minutes * 60.0;
  
  return (hoursToSeconds + minutesToSeconds + seconds) / 86400.0;
}

#pragma mark - String
namespace String {

static std::string toTitleCase(const std::string& str);

}





#pragma mark - Color
namespace Color {

static ofColor hexToColor(const std::string & hex);

static std::vector<ofColor> fromString(const std::string & colors)
{
  return utils::Array::transform<ofColor>(ofSplitString(colors, "-"), &Color::hexToColor);
}

static std::vector<ofColor> fromCoolors(const std::string & URL)
{
  return fromString(ofSplitString(URL, "/", true, true).back());
}

static ofFloatColor proceduralPalette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
  glm::vec3 color = a + b * cos(TWO_PI * (c * ofWrap(t, 0.0, 1.0) + d));
  return ofFloatColor(color.r, color.g, color.b);
}

static ofFloatColor rainbowPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.33, 0.67)); }
static ofFloatColor metallicPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.1, 0.2)); }

static ofMesh getGradientMesh(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0);
static ofMesh getGradientMeshVertical(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0);

}



#pragma mark - Performance
namespace Performance {

template<typename Func, typename... Args>
unsigned int measure(Func&& func, Args&&... args);

}


#pragma mark - Vector
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
inline static glm::vec2 fromAngle(float angle) { return fromRadians(ofDegToRad(angle)); }

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

static glm::vec2 getRotatedAroundOrigin(const glm::vec2 & point, const glm::vec2 & origin, float radians);

inline static void rotateAroundOrigin(glm::vec2 & point, const glm::vec2 & origin, float radians) { point = getRotatedAroundOrigin(point, origin, radians); }

inline static glm::vec2 getRotated(const glm::vec2 & v, float radians)
{
  return glm::vec2(v.x * cos(radians) - v.y * sin(radians), v.x * sin(radians) + v.y * cos(radians));
}

template<typename T> inline static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }
template<typename T> inline static T getLimited(const T & v, float max) { return glm::min(v, glm::normalize(v) * max); }

}



#pragma mark - Files
namespace Files {

template<typename F>
void traverse(const std::string & path, const F& func);

}

}}}
