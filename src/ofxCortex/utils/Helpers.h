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
#include "ofxCortex/types/Box.h"


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

static glm::vec2 randomInRectangle(const ofRectangle & rect)
{
  return { (int) ofRandom(rect.getLeft(), rect.getRight()), (int) ofRandom(rect.getTop(), rect.getBottom()) };
}

static bool isInsideBox(const ofBoxPrimitive & box, const glm::vec3 & p)
{
  return (p.x >= 0.0f && p.x <= box.getWidth()) && (p.y >= 0.0f && p.y <= box.getHeight()) && (p.z >= 0.0f && p.z <= box.getDepth());
}

static bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R)
{
  static const float EPSILON = 0.00000001;
  
  const glm::vec3 & V1 = face.getVertex(0);
  const glm::vec3 & V2 = face.getVertex(1);
  const glm::vec3 & V3 = face.getVertex(2);

  glm::vec3 e1, e2; //Edge1, Edge2
  glm::vec3 P, Q, T;
  float det, inv_det, u, v;
  float t;

  //Find vectors for two edges sharing V1
  e1 = V2 - V1;
  e2 = V3 - V1;
  //Begin calculating determinant - also used to calculate u parameter
  P = glm::cross(D, e2); //D.getCrossed(e2);
  //if determinant is near zero, ray lies in plane of triangle
  det = glm::dot(e1, P); //e1.dot(P);
  //NOT CULLING
  if(det > -EPSILON && det < EPSILON){
    return false;
  }
  inv_det = 1.f / det;

  //calculate distance from V1 to ray origin
  T = O - V1;

  //Calculate u parameter and test bound
  u = glm::dot(T, P) * inv_det; //T.dot(P) * inv_det;
  //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f){
    return false;
  }

  //Prepare to test v parameter
  Q = glm::cross(T, e1); //T.getCrossed(e1);

  //Calculate V parameter and test bound
  v = glm::dot(D, Q) * inv_det; //D.dot(Q) * inv_det;
  //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f){
    return false;
  }

  t = glm::dot(e2, Q) * inv_det; //e2.dot(Q) * inv_det;

  if(t > EPSILON){ //ray intersection
    R = O + t * D; // store intersection point
    return true;
  }

  // No hit, no win
  return false;
}

static bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh)
{
  static bool warning_given;
  if (mesh.getMode() != OF_PRIMITIVE_TRIANGLES)
  {
    if (!warning_given)
    {
      ofLogWarning("isInsideMesh") << "ofMesh mode should be OF_PRIMITIVE_TRIANGLES";
      warning_given = true;
    }
    return false;
  }
  
  glm::vec3 foundIntersection;
  std::vector<glm::vec3> intersections;
  glm::vec3 direction = glm::sphericalRand(1);
  
  for (auto & face : mesh.getUniqueFaces())
  {
    if (rayTriangleIntersection(face, p, direction, foundIntersection)) intersections.push_back(foundIntersection);
  }
  
  std::vector<glm::vec3> unique_intersections;
  std::unique_copy(intersections.begin(), intersections.end(), std::back_inserter(unique_intersections));
  
  return unique_intersections.size() % 2 == 1;
}

static std::vector<glm::vec3> pointsInsideMesh(const std::vector<glm::vec3> & points, const ofMesh & mesh)
{
  std::vector<glm::vec3> output;
  std::copy_if(points.begin(), points.end(), back_inserter(output), [&mesh](const glm::vec3 & p) { return isInsideMesh(p, mesh); });
  return output;
}

static ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh)
{
  glm::vec3 min, max;
  const std::vector<glm::vec3> vertices = mesh.getVertices();
  
  min.x = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.x < b.x; })->x;
  min.y = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.y < b.y; })->y;
  min.z = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.z < b.z; })->z;
  
  max.x = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.x < b.x; })->x;
  max.y = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.y < b.y; })->y;
  max.z = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.z < b.z; })->z;
  
  glm::vec3 size = max - min;
  
  return ofxCortex::core::types::Box(min, size.x, size.y, size.z);
}

static ofRectangle getImageRectangle(const ofBaseDraws & image)
{
  return ofRectangle(0, 0, image.getWidth(), image.getHeight());
}

static void drawTexCoordRectangle(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f)
{
  static ofMesh mesh;
  
  if (!mesh.hasVertices())
  {
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.addVertex(glm::vec3(x, y, 0));
    mesh.addTexCoord(glm::vec2(0, 0));
    
    mesh.addVertex(glm::vec3(x + w, y, 0));
    mesh.addTexCoord(glm::vec2(1, 0));
    
    mesh.addVertex(glm::vec3(x, y + h, 0));
    mesh.addTexCoord(glm::vec2(0, 1));
    
    mesh.addVertex(glm::vec3(x + w, y + h, 0));
    mesh.addTexCoord(glm::vec2(1, 1));
    
//    mesh.addTriangle(0, 1, 2);
//    mesh.addTriangle(2, 3, 0);
  }
  
  ofPushMatrix();
//  ofTranslate(x, y);
//  ofScale(w / 100.0f, h / 100.0f);
  mesh.draw();
  ofPopMatrix();
}

static void drawTexCoordRectangle(const ofRectangle & rect) {
  drawTexCoordRectangle(rect.x, rect.y, rect.width, rect.height);
}

static void drawAxis(const glm::vec3 & position, float scale = 10.0f)
{
  ofPushMatrix();
  {
    ofTranslate(position);
    ofScale(scale);
    
    ofPushStyle();
    {
      ofFill();
      
      ofSetColor(ofColor::tomato);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), 0.05);
      ofDrawBitmapString("X", 1.1, -0.025, -0.025);
      
      ofSetColor(ofColor::springGreen);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 0.05);
      ofDrawBitmapString("Y", -0.025, 1.1, -0.025);
      
      ofSetColor(ofColor::dodgerBlue);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 0.05);
      ofDrawBitmapString("Z", -0.025, -0.025, 1.1);
      
      ofSetColor(ofColor::black);
      ofDrawSphere(0.05);
    }
    ofPopStyle();
  }
  ofPopMatrix();
}

namespace Array {
  template<typename T>
  static T randomInVector(std::vector<T> const &v)
  {
      auto it = v.cbegin();
      int random = rand() % v.size();
      std::advance(it, random);

      return *it;
  }

  template<typename T>
  std::vector<T> exclude(const std::vector<T>& input, const std::vector<T>& exclude) {
    std::vector<T> result;
    std::copy_if(input.begin(), input.end(), std::back_inserter(result), [&](const T& element) {
        return std::find(exclude.begin(), exclude.end(), element) == exclude.end();
    });
    return result;
  }

  template<typename T>
  static T randomInVectorExcept(std::vector<T> const &v, const std::vector<T>& except) { return randomInVector(exclude(v, except)); }

  template<typename T>
  static std::vector<T> constructVector(int n, std::function<T(int)> func = [](int index) { return T(); })
  {
    std::vector<T> output(n);
    for (int i = 0; i < n; i++) output[i] = func(i);
    return output;
  }

  template <typename T>
  std::vector<T> randomSubset(const std::vector<T>& v, std::size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::size_t> indices(v.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gen);
    std::partial_sort(indices.begin(), indices.begin() + size, indices.end(),
        [&gen](std::size_t i, std::size_t j) { return gen() < gen(); });
    std::vector<T> result(size);
    std::transform(indices.begin(), indices.begin() + size, result.begin(),
        [&v](std::size_t i) { return v[i]; });
    return result;
  }

  template<typename T>
  void appendVector(std::vector<T> & original, std::vector<T> & appending)
  {
    original.insert(std::end(original), std::begin(appending), std::end(appending));
  }

  int coordinateToIndex(glm::ivec3 coord, glm::ivec3 size) { return coord.x + coord.y * size.x + coord.z * size.x * size.y; }
  int coordinateToIndex(glm::ivec2 coord, glm::ivec2 size) { return coord.x + coord.y * size.x; }

  glm::ivec3 indexToCoordinate(int index, glm::ivec3 size)
  {
    if (index < 0 || index >= size.x * size.y * size.z)
    {
  //    std::cout << "ofxCortex::core::utils::Array::indexToCoordinate(): Index out of bounds [0-" << (size.x * size.y * size.z) << "]. Clamping to range." << std::endl;
        index = CLAMP(index, 0, size.x * size.y * size.z - 1);
    }
    
    glm::ivec3 coord;
    coord.x = index % size.x;
    coord.y = (index / size.x) % size.y;
    coord.z = index / (size.x * size.y);
    
    return coord;
  }

  glm::ivec2 indexToCoordinate(int index, glm::ivec2 size)
  {
    if (index < 0 || index >= size.x * size.y)
    {
//      ofLogWarning("ofxCortex::core::utils::Array::indexToCoordinate") << "Index out of bounds [0-" << size.x * size.y << "]. Clamping to the range.";
      index = CLAMP(index, 0, size.x * size.y - 1);
    }
    
    glm::ivec2 coord;
    coord.x = index % size.x;
    coord.y = index / size.x;
    
    return coord;
  }
}

namespace Performance {
  template<typename Func, typename... Args>
  unsigned int measure(Func&& func, Args&&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    std::forward<Func>(func)(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return static_cast<unsigned int>(duration.count());
  }
}

namespace Vector {

  static glm::vec2 random2D(float radius = 1.0)
  {
    return glm::circularRand(MAX(radius, std::numeric_limits<float>::epsilon()));
  }

  static glm::vec3 random3D(float radius = 1.0)
  {
    return glm::sphericalRand(MAX(radius, std::numeric_limits<float>::epsilon()));
  }

  static glm::vec3 to3D(const glm::vec2 & v, float z = 0.0f) { return { v.x, v.y, z }; }
  static std::vector<glm::vec3> to3D(const std::vector<glm::vec2> & vec, float z = 0.0f)
  {
    std::vector<glm::vec3> output;
    for (const glm::vec2 & p : vec) output.push_back({ p.x, p.y, z });
    return output;
  }

  template<typename T>
  static void translate(std::vector<T> & v, const T & p) { std::for_each(v.begin(), v.end(), [&p](T & e) { e += p; }); }

  static glm::vec2 fromRadians(float angle) { return { cosf(angle), sinf(angle) }; }
  static glm::vec2 fromDegrees(float angle) { return fromRadians(ofDegToRad(angle)); }
  static glm::vec2 fromAngle(float angle) { return fromRadians(ofDegToRad(angle)); }

  static float toRadians(const glm::vec2 & v) { return glm::atan(v.y, v.x); }
  static float toDegrees(const glm::vec2 & v) { return ofRadToDeg(toRadians(v)); }
  static float toAngle(const glm::vec2 & v) { return toDegrees(v); }

  template<typename T> static void flipX(T & v) { v.X * -1.0f; }
  template<typename T> static void flipY(T & v) { v.y * -1.0f; }

  template<typename T> static T getFlippedY(const T & v) { return v * T(1, -1, 1); }

  static glm::vec2 getRotatedAroundOrigin(const glm::vec2 & point, const glm::vec2 & origin, float radians)
  {
    const float co = cos(radians);
    const float si = sin(radians);
    
    float nx = (co * (point.x - origin.x)) + (si * (point.y - origin.y)) + origin.x;
    float ny = (co * (point.y - origin.y)) - (si * (point.x - origin.x)) + origin.y;
    return { nx, ny };
  }

  static void rotateAroundOrigin(glm::vec2 & point, const glm::bvec2 & origin, float radians)
  {
    point = getRotatedAroundOrigin(point, origin, radians);
  }

  template<typename T>
  static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }

}

}}}
