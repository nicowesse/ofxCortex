#include "Line.h"

namespace ofxCortex { namespace core { namespace graphics {

ofPath Line::getLinePath(const ofPolyline & source, float thickness, ClipperLib::JoinType jointype, ClipperLib::EndType endtype)
{
  return polysToPath(Clipper::getOffsets({ source }, thickness, jointype, endtype));
}

void Line::drawPolyline(const ofPolyline & source, float thickness, ofFloatColor color, ClipperLib::JoinType jointype, ClipperLib::EndType endtype)
{
  if (ofIsFloatEqual(thickness, 1.0f))
  {
    ofPushStyle();
    ofSetColor(color);
    source.draw();
    ofPopStyle();
    
    return;
  }
  
  ofPath path = polysToPath(Clipper::getOffsets({ source }, thickness, jointype, endtype));
  path.setFillColor(color);
  path.draw();
}

void Line::catmullRom(ofPolyline & source, int iterations)
{
  if (source.size() < 3) {
    ofLogNotice("PolylineUtils::catmullRom()") << "Source needs at least 3 vertices.";
    return;
  }
  
  iterations = std::max(1, iterations);
  
  std::vector<glm::vec3> points = source.getVertices();
  
  glm::vec3 extrapolated = points[1] - points[0];
  points.insert(points.begin(), points[0] - extrapolated);
  
  extrapolated = points[points.size() - 1] - points[points.size() - 2];
  points.insert(points.end(), points[points.size() - 1] + extrapolated);
  
  std::vector<glm::vec3> spline;
  for (int i = 0; i < points.size() - 3; i++)
  {
    for (int p = 0; p < iterations; p++)
    {
      spline.push_back(glm::catmullRom(points[i], points[i+1], points[i+2], points[i+3], p / (float) iterations));
    }
  }
  spline.push_back(points[points.size() - 2]);
  
  source = ofPolyline(spline);
}

ofPolyline Line::getCatmullRom(const ofPolyline & source, int iterations)
{
  ofPolyline output = source;
  catmullRom(output, iterations);
  return output;
}

void Line::chaikin(ofPolyline & source, int iterations, float tension)
{
  if (source.size() < 3) {
    ofLogNotice("Line::chaikin()") << "Source needs at least 3 vertices.";
    return;
  }
  
  iterations = std::max(1, iterations);
  tension = ofClamp(tension, 0.0f, 1.0f);
  bool closed = source.isClosed();
  
  float cuttingDistance = 0.05f + (tension * 0.4f);
  std::vector<glm::vec3> points = source.getVertices();
  
  auto getSmoothedPoints = [](const std::vector<glm::vec3> &points, float cuttingDistance, bool closed) -> std::vector<glm::vec3>
  {
    if (points.size() < 3) return points;
    std::vector<glm::vec3> output;
    size_t numPoints = (closed) ? points.size(): points.size() - 1;
    
    
    if (!closed) output.push_back(points[0]);
    
    for (int i = 0; i < numPoints; i++)
    {
      output.push_back((1.0f - cuttingDistance) * points[i % points.size()] + cuttingDistance * points[(i + 1) % points.size()]);
      output.push_back(cuttingDistance * points[i % points.size()] + (1.0f - cuttingDistance) * points[(i + 1) % points.size()]);
    }
    
    if (!closed) output.push_back(points[points.size() - 1]);
    
    return output;
  };
  
  for (int i = 0; i < iterations; i++) { points = getSmoothedPoints(points, cuttingDistance, closed); }
  
  source = ofPolyline(points);
}

ofPolyline Line::getChaikin(const ofPolyline & source, int iterations, float tension)
{
  ofPolyline output = source;
  chaikin(output, iterations, tension);
  return output;
}

ofPolyline Line::getRoundedPolyline(const ofPolyline & source, const std::vector<float> & radiuses)
{
  const std::vector<glm::vec3> & points = source.getVertices();
  size_t len = points.size();
  
  ofPolyline output;
  
  bool isClosed = source.isClosed();
  
  if (!isClosed) output.addVertex(points.front());
  for (int i = !isClosed; i < len - !isClosed; i++)
  {
    glm::vec3 P = points[utils::modulo(i - 1, len)]; // Previous
    glm::vec3 C = points[utils::modulo(i + 0, len)]; // Current
    glm::vec3 N = points[utils::modulo(i + 1, len)]; // Next
    
    if (!source.isClosed() && i == 0) { output.addVertex(C); continue; }
    
    glm::vec3 CP = P - C;
    glm::vec3 CN = N - C;
    
    glm::vec3 normCP = glm::normalize(CP);
    glm::vec3 normCN = glm::normalize(CN);
    
    glm::vec3 perpCN = ofxCortex::core::utils::Vector::to3D(ofxCortex::core::utils::Vector::getRotated(normCN, -PI / 2));
    glm::vec3 perpCP = ofxCortex::core::utils::Vector::to3D(ofxCortex::core::utils::Vector::getRotated(normCP, PI / 2));
    
    float angle = utils::Vector::angleBetween(CP, CN);
    
    if (ofIsFloatEqual(abs(angle), (float) PI)) { output.addVertex(C); continue; }
//    if (fmod(abs(angle), (float) PI) == 0.0) { output.addVertex(C); continue; }

    glm::vec3 accelDir = glm::normalize(normCP + normCN);
    
    float radDirection = ofSign(glm::dot(accelDir, -perpCP));
    bool drawDirection = radDirection == 1;
    
    int radiusIndex = ofClamp(i, 0, radiuses.size() - 1);
    float radius = radiuses[radiusIndex];

    float halfAngle = angle / 2;
    float lenOut = abs((cos(halfAngle) * radius) / sin(halfAngle));
    float cRadius = radius;
    
    float shortestEdgeLength = std::min(glm::length(CP) / 2, glm::length(CN) / 2);

    // Special part A
    if (lenOut > shortestEdgeLength) {
      lenOut = shortestEdgeLength;
      cRadius = abs((lenOut * sin(halfAngle)) / cos(halfAngle));
    }

    glm::vec3 center;
    center.x = C.x + normCN.x * lenOut - normCN.y * cRadius * radDirection;
    center.y = C.y + normCN.y * lenOut + normCN.x * cRadius * radDirection;
    
    float startAngle = utils::Vector::toDegrees(perpCP * radDirection);
    float endAngle = utils::Vector::toDegrees(perpCN * radDirection);
    
    float actualAngle = abs(ofWrapDegrees(endAngle - startAngle));
    int segments = round((actualAngle / 180) * 64);
    
    output.arc(center, cRadius, cRadius, startAngle, endAngle, drawDirection, segments);
  }
  
  if (source.isClosed()) output.close();
  else output.addVertex(points.back());
  
  return output;
}

ofPolyline Line::getBezierRoundedPolyline(const ofPolyline & source, const std::vector<float> & radiuses)
{
  const std::vector<glm::vec3> & points = source.getVertices();
  size_t len = points.size();
  
  ofPolyline output;
  
  auto angleBetween = [](const glm::vec3 & a, const glm::vec3 & b) -> double
  {
    const float magSqMult = glm::length2(a) * glm::length2(b);
    
    if (magSqMult == 0.0) return NAN;
    
    const glm::vec3 u = glm::cross(a, b);
    
    float angle = atan2(glm::length(u), glm::dot(a, b)) * ofSign(u.z || 1);
    return angle;
  };
  
  bool isClosed = source.isClosed();
  
  if (!isClosed) output.addVertex(points.front());
  for (int i = !isClosed; i < len - !isClosed; i++) {
    glm::vec3 a = points[utils::modulo(i - 1, len)];
    glm::vec3 b = points[utils::modulo(i + 0, len)];
    glm::vec3 c = points[utils::modulo(i + 1, len)];
    glm::vec3 ba = glm::normalize(a - b); //a.copy().sub(b).normalize();
    glm::vec3 bc = glm::normalize(c - b); //c.copy().sub(b).normalize()
    
    // Points in the direction the corner is accelerating towards
    glm::vec3 normal = glm::normalize(ba + bc); //ba.copy().add(bc).normalize()
    
    // Shortest angle between the two edges
    float theta = angleBetween(ba, bc); //ba.angleBetween(bc)
    
    int radiusIndex = ofClamp(i, 0, radiuses.size() - 1);
    float radius = radiuses[radiusIndex];
    
    // Find the circle radius that would cause us to round off half
    // of the shortest edge. We leave the other half for neighbouring
    // corners to potentially cut.
    float maxR = std::min(glm::distance(a, b), glm::distance(c, b)) / 2.0 * abs(sin(theta / 2));
    float cornerR = std::min(radius, maxR);
    // Find the distance away from the corner that has a distance of
    // 2*cornerR between the edges
    float distance = abs(cornerR / sin(theta / 2));
    
    // Approximate an arc using a cubic bezier
    glm::vec3 c1 = b + (ba * distance); //b.copy().add(ba.copy().mult(distance))
    glm::vec3 c2 = b + (bc * distance); //b.copy().add(bc.copy().mult(distance))
    float bezierDist = 0.5523; // https://stackoverflow.com/a/27863181
    
    glm::vec3 p1 = c1 - (ba * 2 * cornerR * bezierDist); //c1.copy().sub(ba.copy().mult(2*cornerR*bezierDist))
    glm::vec3 p2 = c2 - (bc * 2 * cornerR * bezierDist); //c2.copy().sub(bc.copy().mult(2*cornerR*bezierDist))
    output.addVertex(c1);
    output.bezierTo(p1, p2, c2);
  }
  if (isClosed) output.close();
  else output.addVertex(points.back());
  
  return output;
}

ofPath Line::polysToPath(const std::vector<ofPolyline> & polylines) {
  ofPath path;
  for(int outline = 0; outline < polylines.size(); ++outline) {
    for (int i = 0; i < polylines[outline].getVertices().size(); i++){
      if ( i == 0 )
        path.moveTo(polylines[outline].getVertices()[i].x,polylines[outline].getVertices()[i].y);
      else
        path.lineTo(polylines[outline].getVertices()[i].x,polylines[outline].getVertices()[i].y);
    }
    if (polylines[outline].isClosed()) path.close();
  }
  return path;
}

std::vector<ofPolyline> Line::getOffsets(const ofPolyline & source, std::vector<float> offsets, ClipperLib::JoinType jointype,
                                     ClipperLib::EndType endtype)
{
  std::vector<ofPolyline> output;
  
  if (source.size() == 0) return output;
  
  for (float offset : offsets)
  {
    auto lines = Clipper::getOffsets({ source }, offset, jointype, endtype);
    output.insert(output.end(), lines.begin(), lines.end());
  }
  
  return output;
}

ofPolyline Line::getOffset(const ofPolyline & source, float offset, ClipperLib::JoinType jointype, ClipperLib::EndType endtype)
{
  if (source.size() == 0) return ofPolyline();
  
  auto lines = getOffsets(source, std::vector<float>{ offset }, jointype, endtype);
  
  return (lines.size()) ? lines[0] : source;
}

void Line::offset(ofPolyline & source, float offset, ClipperLib::JoinType jointype, ClipperLib::EndType endtype)
{
  if (source.size() == 0) return;
  
  auto temp = getOffset(source, offset, jointype, endtype);
  source = temp;
}

ofPolyline Line::getLineSubsection(const ofPolyline & source, float start, float end)
{
  if (source.size() == 0) return source;
  
  float actualStart = MIN(start, end);
  float actualEnd = MAX(start, end);
  
  if (ofIsFloatEqual(actualStart, actualEnd)) return ofPolyline();
  if (ofIsFloatEqual(actualStart, 0.0f) && ofIsFloatEqual(actualEnd, 1.0f)) return source;
  
  ofPolyline output;
  
  float startIndex = source.getIndexAtPercent(actualStart);
  int fillStartIndex = ceil(startIndex);
  float endIndex = source.getIndexAtPercent(actualEnd);
  int fillEndIndex = floor(endIndex) + 1;
  
  std::vector<glm::vec3> filler;
  
  if (source.isClosed() && start > end)
  {
    std::swap(startIndex, endIndex);
    
    std::copy(source.begin() + fillEndIndex, source.end(), back_inserter(filler));
    std::copy(source.begin(), source.begin() + fillStartIndex, back_inserter(filler));
  }
  else std::copy(source.begin() + fillStartIndex, source.begin() + fillEndIndex, back_inserter(filler));
  
  
  output.addVertex(source.getPointAtIndexInterpolated(startIndex));
  output.addVertices(filler);
  output.addVertex(source.getPointAtIndexInterpolated(endIndex));
  
  return output;
}


ofPolyline Line::getInterpolatedPolyline(const ofPolyline & source, const ofPolyline & target, float t){
  if (source.size() != target.size())
  {
    ofLogWarning("Line::getInterpolatedPolyline") << "Source and Target polylines needs equal amount of vertices. Returning source line.."; return source;
  }
  
  ofPolyline output;
  for (int i = 0; i < source.size(); i++) { output.addVertex(glm::mix(source[i], target[i], t)); }
  return output;
}

void Line::scribbleLine(ofPolyline & source, float resolution, float amplitude)
{
  if (source.size() < 2) return;
  
  source = source.getResampledBySpacing(resolution);
  
  for (int i = 0; i < source.size(); i++)
  {
    glm::vec3 normal = source.getNormalAtIndex(i);
    float t = (float) i / (source.size() - 1);
    float offset = ofSignedNoise(t, i) * amplitude;
    source[i] += normal * offset;
  }
}
  
ofPolyline Line::getScribbledLine(const ofPolyline & source, float resolution, float amplitude)
{
  ofPolyline output = source;
  scribbleLine(output, resolution, amplitude);
  return output;
}

ofPolyline Line::getReversed(const ofPolyline &source)
{
  ofPolyline temp;
  for (int i = source.size() - 1; i >= 0; i--)
  {
    temp.addVertex(source[i]);
  }
  return temp;
}

ofPolyline Line::fromRectangle(const ofRectangle & rect)
{
  ofPolyline line;
  
  line.addVertex(rect.getTopLeft());
  line.addVertex(rect.getTopRight());
  line.addVertex(rect.getBottomRight());
  line.addVertex(rect.getBottomLeft());
  line.close();
  
  return line;
}

void Line::drawGradientLine(const ofPolyline & line, const ofColor & fromColor, const ofColor & toColor, float resolution)
{
  ofPolyline subline = line.getResampledBySpacing(resolution);
  
  ofPushStyle();
  for (int i = 0; i < subline.size() - 1; i++)
  {
    float t = (float) i / (subline.size() - 1);
    
    ofSetColor(fromColor.getLerped(toColor, t));
    ofDrawLine(subline[i], subline[i+1]);
  }
  ofPopStyle();
}

ofPolyline Line::getSimplifiedPolyline(const ofPolyline& source, float epsilon) 
{
  if (source.size() < 3) {
    // Polyline with less than 3 points cannot be simplified
    return source;
  }
  
  std::vector<glm::vec3> points = source.getVertices();
  std::vector<glm::vec3> simplifiedPoints;
  simplifyRDP(points, 0, points.size() - 1, epsilon, simplifiedPoints, source.isClosed());
  
  ofPolyline simplifiedPolyline(simplifiedPoints);
  
  if (source.isClosed()) simplifiedPolyline.close();
  
  return simplifiedPolyline;
}

void Line::simplifyRDP(const std::vector<glm::vec3>& points, int startIdx, int endIdx, float epsilon, std::vector<glm::vec3>& simplifiedPoints, bool isClosed)
{
  epsilon = std::max(epsilon, 0.0f);
  
  float dmax = 0;
  int index = 0;
  int n = points.size();

  for (int i = startIdx + 1; i < endIdx; ++i) {
      float d = lineDistance(points[i], points[startIdx], points[endIdx]);
      if (d > dmax) {
          dmax = d;
          index = i;
      }
  }

  if (isClosed && startIdx == 0 && endIdx == points.size() - 1) {
      // If the polyline is closed and the start and end points are being considered, add them to the simplified polyline.
      simplifiedPoints.push_back(points[startIdx]);
      if (dmax > epsilon) {
          simplifyRDP(points, startIdx, index, epsilon, simplifiedPoints, isClosed);
          simplifyRDP(points, index, endIdx, epsilon, simplifiedPoints, isClosed);
      }
      return;
  }

  if (dmax > epsilon) {
      // If the maximum distance is greater than epsilon, recursively simplify both subparts
      simplifyRDP(points, startIdx, index, epsilon, simplifiedPoints, isClosed);
      simplifyRDP(points, index, endIdx, epsilon, simplifiedPoints, isClosed);
  } else {
      // If not, add the start point to the simplified polyline
      simplifiedPoints.push_back(points[startIdx]);

      if (!isClosed || endIdx != points.size() - 2) {
          // If the polyline is open or the end point is not the last point, add the end point to the simplified polyline.
          simplifiedPoints.push_back(points[endIdx]);
      }
  }
}

}}}
