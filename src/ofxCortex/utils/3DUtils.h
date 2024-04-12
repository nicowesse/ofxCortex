#pragma once

#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

#include "ofUtils.h"
#include "ofGraphics.h"
#include "of3dGraphics.h"
#include "of3dUtils.h"
#include "ofVectorMath.h"

#include "ofxCortex/types/Box.h"

namespace ofxCortex { namespace core { namespace utils {

inline static bool isInsideBox(const ofBoxPrimitive & box, const glm::vec3 & p)
{
  return (p.x >= 0.0f && p.x <= box.getWidth()) && (p.y >= 0.0f && p.y <= box.getHeight()) && (p.z >= 0.0f && p.z <= box.getDepth());
}

static bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh);

inline static std::vector<glm::vec3> pointsInsideMesh(const std::vector<glm::vec3> & points, const ofMesh & mesh)
{
  std::vector<glm::vec3> output;
  std::copy_if(points.begin(), points.end(), back_inserter(output), [&mesh](const glm::vec3 & p) { return isInsideMesh(p, mesh); });
  return output;
}

static ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh);

static bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R);



}}}
