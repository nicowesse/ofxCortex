#pragma once

#include "ofMath.h"

namespace ofxCortex { namespace core { namespace utils {

namespace Geometry {

inline static bool lineLineIntersection(const glm::vec2& p1, const glm::vec2& d1, const glm::vec2& p2, const glm::vec2& d2, glm::vec2& intersection) {
  // Calculate the components of the direction vectors
  float x1 = p1.x, y1 = p1.y;
  float x2 = p2.x, y2 = p2.y;
  float dx1 = d1.x, dy1 = d1.y;
  float dx2 = d2.x, dy2 = d2.y;
  
  // Calculate the denominator
  float denom = dx1 * dy2 - dy1 * dx2;
  
  // If denom is 0, the lines are parallel
  if (fabs(denom) < 1e-6) {
    return false; // No intersection
  }
  
  // Calculate the parameters t and u for the parametric equation
  float t = ((x2 - x1) * dy2 - (y2 - y1) * dx2) / denom;
  float u = ((x2 - x1) * dy1 - (y2 - y1) * dx1) / denom;
  
  // Check if the intersection point is within the direction of the rays (optional)
  // In this case, it's checking if the intersection happens in the positive direction of both rays
  if (t >= 0 && u >= 0) {
    intersection.x = x1 + t * dx1;
    intersection.y = y1 + t * dy1;
    return true; // Intersection found
  }
  
  return false; // No intersection in the positive direction of both rays
}

inline static std::pair<glm::vec2, float> threePointCircle(const glm::vec2 & P1, const glm::vec2 & P2, const glm::vec2 & P3)
{
  float x1 = P1.x, y1 = P1.y;
  float x2 = P2.x, y2 = P2.y;
  float x3 = P3.x, y3 = P3.y;
  
  float x12 = x1 - x2;
  float x13 = x1 - x3;
  float y12 = y1 - y2;
  float y13 = y1 - y3;
  float y31 = y3 - y1;
  float y21 = y2 - y1;
  float x31 = x3 - x1;
  float x21 = x2 - x1;
  
  // x1^2 - x3^2 and y1^2 - y3^2
  float sx13 = x1 * x1 - x3 * x3;
  float sy13 = y1 * y1 - y3 * y3;
  float sx21 = x2 * x2 - x1 * x1;
  float sy21 = y2 * y2 - y1 * y1;
  
  float denominator = 2 * ((y31 * x12) - (y21 * x13));
  
  float f = (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / denominator;
  float g = (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / denominator;
  
  float c = -(x1 * x1 + y1 * y1 + 2 * g * x1 + 2 * f * y1);
  
  // Center (h, k) and radius
  float h = -g;
  float k = -f;
  float r = sqrt(h * h + k * k - c);
  
  return { glm::vec2(h, k), r };
}

inline static void appendThreePointArc(ofPath & path, const glm::vec2 & C, const glm::vec2 & A, const glm::vec2 & B, float radius, bool clockwise) {
  glm::vec2 CA = glm::normalize(A - C);
  glm::vec2 CB = glm::normalize(B - C);
  
  float angleA = utils::Vector::toAngle(CA);
  float angleB = utils::Vector::toAngle(CB);
  
  path.arc(C, radius, radius, angleA, angleB, clockwise);
}

inline static std::vector<glm::vec2> getCircleIntersections(const glm::vec2& C1, float R1,
                                                            const glm::vec2& C2, float R2) {
  std::vector<glm::vec2> intersections;
  
  float dist = glm::distance(C1, C2);
  
  // Check if the circles intersect
  if (dist > R1 + R2 || dist < fabs(R1 - R2)) {
    ofLogWarning("getCircleIntersections") << "No intersection: circles are too far apart or one is contained within the other";
    return intersections;
  }
  
  // Calculate the point where the line through the circle intersection points crosses the line between the circle centers
  float a = (R1 * R1 - R2 * R2 + dist * dist) / (2.0f * dist);
  glm::vec2 p = C1 + a * (C2 - C1) / dist;
  
  // Calculate the distance from point p to the intersection points
  float h = sqrt(R1 * R1 - a * a);
  
  // Offset for the intersection points
  glm::vec2 offset = h * glm::vec2(C2.y - C1.y, C1.x - C2.x) / dist;
  
  // Two intersection points
  intersections.push_back(p + offset);
  intersections.push_back(p - offset);
  
  return intersections;
}

inline static std::vector<glm::vec2> getCircleLineIntersections(const glm::vec2& center, float radius,
                                                                const glm::vec2& lineStart, const glm::vec2& lineEnd) {
  std::vector<glm::vec2> intersections;
  
  // Direction vector of the line
  glm::vec2 d = lineEnd - lineStart;
  
  // Vector from the circle center to the line start
  glm::vec2 f = lineStart - center;
  
  float a = glm::dot(d, d);
  float b = 2 * glm::dot(f, d);
  float c = glm::dot(f, f) - radius * radius;
  
  // Solving the quadratic equation: at^2 + bt + c = 0
  float discriminant = b * b - 4 * a * c;
  
  if (discriminant < 0) {
    // No intersection
    return intersections;
  }
  
  // Two solutions for t (line parameter)
  discriminant = sqrt(discriminant);
  
  float t1 = (-b - discriminant) / (2 * a);
  float t2 = (-b + discriminant) / (2 * a);
  
  // Check if t1 and t2 are within the line segment (0 <= t <= 1)
  if (t1 >= 0 && t1 <= 1) {
    intersections.push_back(lineStart + t1 * d);
  }
  if (t2 >= 0 && t2 <= 1) {
    intersections.push_back(lineStart + t2 * d);
  }
  
  return intersections;
}

// Tangents
struct TangentPoints {
  glm::vec2 T1;
  glm::vec2 T2;
};

inline static std::optional<TangentPoints> findTangentPoints(const glm::vec2 & center, float radius, const glm::vec2 & point) {
  glm::vec2 CA = point - center;
  float d = glm::length(CA);
  
  // If the point is inside the circle, return no tangent points
  if (d < radius) {
    ofLogWarning("findTangentPoints") << "Point is inside circle.";
    return std::nullopt;
  }
  
  float l = std::sqrt(d * d - radius * radius);
  glm::vec2 unitCA = CA / d;
  
  // Perpendicular vector
  glm::vec2 perpCA(-unitCA.y, unitCA.x);
  
  glm::vec2 T1 = center + radius * (unitCA * radius + perpCA * l) / d;
  glm::vec2 T2 = center + radius * (unitCA * radius - perpCA * l) / d;
  
  return TangentPoints{T1, T2};
}

inline static std::vector<TangentPoints> findCircleTangents(const glm::vec2& C1, float r1, const glm::vec2& C2, float r2) {
  std::vector<TangentPoints> tangents;
  glm::vec2 d = C2 - C1;
  float dLength = glm::length(d);
  
  // Ensure circles are not overlapping
  if (dLength < std::abs(r1 - r2) || dLength == 0.0f) {
    // No tangents possible if the circles are overlapping or coincident
    
    ofLogWarning("findCircleTangents") << "No tangents possible if the circles are overlapping or coincident.";
    return tangents;
  }
  
  // Calculate external tangents
  for (int sign : {-1, 1}) {
    float rdiff = r2 - sign * r1;
    float l = std::sqrt(dLength * dLength - rdiff * rdiff);
    glm::vec2 unitD = d / dLength;
    glm::vec2 perpD(-unitD.y, unitD.x);
    
    glm::vec2 P1 = C1 + r1 * (unitD * rdiff + perpD * l) / dLength;
    glm::vec2 P2 = C2 + r2 * (unitD * rdiff + perpD * l * sign) / dLength;
    
    tangents.push_back(TangentPoints{P1, P2});
  }
  
  // Calculate internal tangents if the circles do not overlap
  if (dLength > r1 + r2) {
    for (int sign : {-1, 1}) {
      float rsum = r1 + sign * r2;
      float l = std::sqrt(dLength * dLength - rsum * rsum);
      glm::vec2 unitD = d / dLength;
      glm::vec2 perpD(-unitD.y, unitD.x);
      
      glm::vec2 P1 = C1 + r1 * (unitD * rsum + perpD * l) / dLength;
      glm::vec2 P2 = C2 + r2 * (unitD * rsum - perpD * l * sign) / dLength;
      
//      tangents.push_back(TangentPoints{P1, P2});
    }
  }
  
  return tangents;
}

inline static glm::vec2 closestPointOnLine(const glm::vec2 & P, const glm::vec2 & A, const glm::vec2 & B){
  glm::vec2 D = glm::normalize(B - A);
  return A + D * glm::dot(P - A, D);
}



}}}}
