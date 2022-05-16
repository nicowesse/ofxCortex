#pragma once

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "of3dGraphics.h"
#include "ofGraphics.h"
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
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


namespace Vector {
static glm::vec2 random2D(float radius = 1.0)
{
  return glm::circularRand(radius);
}

static glm::vec3 random3D(float radius = 1.0)
{
  return glm::sphericalRand(MAX(radius, 0.001));
}

static glm::vec3 to3D(const glm::vec2 & v, float z = 0.0f) { return { v.x, v.y, z }; }
static std::vector<glm::vec3> to3D(const std::vector<glm::vec2> & vec, float z = 0.0f)
{
  std::vector<glm::vec3> output;
  for (const glm::vec2 & p : vec) output.push_back({ p.x, p.y, z });
  return output;
}

template<typename T>
static void translate(std::vector<T> & v, const T & p)
{
  std::for_each(v.begin(), v.end(), [&p](T & e) { e += p; });
}

static glm::vec2 fromAngle(float angle) { return { cosf(angle), sinf(angle) }; }
static float toAngle(const glm::vec2 & v) { return glm::atan(v.y, v.x); }

template<typename T>
static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }

}

}}}
