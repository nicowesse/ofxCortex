#include "3DUtils.h"


namespace ofxCortex { namespace core { namespace utils {

bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R)
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

bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh)
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

ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh)
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


}}}
