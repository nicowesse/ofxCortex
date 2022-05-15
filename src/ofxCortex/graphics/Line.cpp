#include "Line.h"

namespace ofxCortex { namespace graphics {

ofPath Line::getLinePath(const ofPolyline & source, float thickness, ClipperLib::JoinType jointype, ClipperLib::EndType endtype)
{
  return polysToPath(ofx::Clipper::getOffsets({ source }, thickness, jointype, endtype));
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
  
  ofPath path = polysToPath(ofx::Clipper::getOffsets({ source }, thickness, jointype, endtype));
  path.setFillColor(color);
  path.draw();
}

void Line::catmullRom(ofPolyline & target, const ofPolyline & source, int iterations)
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
  
  target = ofPolyline(spline);
}

ofPolyline Line::getCatmullRom(const ofPolyline & source, int iterations)
{
  ofPolyline output;
  catmullRom(output, source, iterations);
  return output;
}

void Line::chaikin(ofPolyline & target, const ofPolyline & source, int iterations, float tension)
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
    int numPoints = (closed) ? points.size(): points.size() - 1;
    
    
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
  
  target = ofPolyline(points);
}

ofPolyline Line::getChaikin(const ofPolyline & source, int iterations, float tension)
{
  ofPolyline output;
  chaikin(output, source, iterations, tension);
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

ofPolyline Line::getLineSubsection(const ofPolyline & source, float start, float end)
{
  float actualStart = MIN(start, end);
  float actualEnd = MAX(start, end);
  
  if (ofIsFloatEqual(actualStart, actualEnd)) return ofPolyline();
  if (ofIsFloatEqual(actualStart, 0.0f) && ofIsFloatEqual(actualEnd, 1.0f)) return source;
  
  float startIndex = source.getIndexAtPercent(actualStart);
  int fillStartIndex = ceil(startIndex);
  float endIndex = source.getIndexAtPercent(actualEnd);
  int fillEndIndex = floor(endIndex) + 1;
  
  std::vector<glm::vec3> filler;
  std::copy(source.begin() + fillStartIndex, source.begin() + fillEndIndex, back_inserter(filler));
  
  ofPolyline output;
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
    float t = (float)i / (source.size() - 1);
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

void Line::reverse(ofPolyline & source)
{
  source = getReversed(source);
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

}}
