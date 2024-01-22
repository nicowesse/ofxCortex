#pragma once

#include "ofxClipper.h"

#include "ofGraphics.h"
#include "ofMath.h"
#include "ofPolyline.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace graphics {

class Line {
  
public:
  typedef ClipperLib::JoinType JoinType;
  typedef ClipperLib::EndType EndType;
  
  // Rendering
  static ofPath getLinePath(const ofPolyline & source, float thickness, ClipperLib::JoinType jointype = ClipperLib::jtRound, ClipperLib::EndType endtype = ClipperLib::etOpenRound);
  static void drawPolyline(const ofPolyline & source, float thickness, ofFloatColor color = ofFloatColor::white, ClipperLib::JoinType jointype = ClipperLib::jtRound, ClipperLib::EndType endtype = ClipperLib::etOpenRound);

  // Interpolations/modifiers
  static void catmullRom(ofPolyline & source, int iterations = 10);
  static ofPolyline getCatmullRom(const ofPolyline & source, int iterations = 10);
  
  static void chaikin(ofPolyline & source, int iterations = 4, float tension = 0.5f);
  static ofPolyline getChaikin(const ofPolyline & source, int iterations = 4, float tension = 0.5f);
  
  static void scribbleLine(ofPolyline & source, float resolution, float amplitude);
  static ofPolyline getScribbledLine(const ofPolyline & source, float resolution, float amplitude);
  
  static ofPolyline getRoundedPolyline(const ofPolyline & source, const std::vector<float> & radiuses);
  static ofPolyline getRoundedPolyline(const ofPolyline & source, float radius) { return getRoundedPolyline(source, std::vector<float>{ radius });};
  
  static ofPolyline getBezierRoundedPolyline(const ofPolyline & source, const std::vector<float> & radiuses);
  static ofPolyline getBezierRoundedPolyline(const ofPolyline & source, float radius) { return getBezierRoundedPolyline(source, std::vector<float>{ radius }); };
  
  // Helpers
  static ofPath polysToPath(const std::vector<ofPolyline> & polylines);
  
  static std::vector<ofPolyline> getOffsets(const ofPolyline & source, std::vector<float> offsets, ClipperLib::JoinType jointype = ClipperLib::jtSquare, ClipperLib::EndType endtype = ClipperLib::etOpenSquare);
  static ofPolyline getOffset(const ofPolyline & source, float offset, ClipperLib::JoinType jointype = ClipperLib::jtSquare, ClipperLib::EndType endtype = ClipperLib::etOpenSquare);
  static void offset(ofPolyline & source, float offset, ClipperLib::JoinType jointype = ClipperLib::jtSquare, ClipperLib::EndType endtype = ClipperLib::etOpenButt);
  
  static ofPolyline getLineSubsection(const ofPolyline & source, float start, float end);
  static ofPolyline getInterpolatedPolyline(const ofPolyline & source, const ofPolyline & target, float t);
  
  static void reverse(ofPolyline & source) { source = getReversed(source); };
  static ofPolyline getReversed(const ofPolyline & source);
  
  static ofPolyline fromRectangle(const ofRectangle & rect);
  
  static void drawGradientLine(const ofPolyline & line, const ofColor & fromColor, const ofColor & toColor, float resolution = 2.0);
  
  static void simplifyPolyline(ofPolyline & source, float epsilon = 0.5) { source = getSimplifiedPolyline(source, epsilon); };
  static ofPolyline getSimplifiedPolyline(const ofPolyline& source, float epsilon = 0.5);
  
private:
  Line() = default;
  
  static void simplifyRDP(const std::vector<glm::vec3>& points, int startIdx, int endIdx, float epsilon, std::vector<glm::vec3>& simplifiedPoints, bool isClosed);

  static float lineDistance(const glm::vec3 & point, const glm::vec3 & start, const glm::vec3 & end) {
    // Calculate the perpendicular distance from a point to a line
    float num = abs((end.y - start.y) * point.x - (end.x - start.x) * point.y + end.x * start.y - end.y * start.x);
    float den = sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
    return num / den;
  }
};

}}}
