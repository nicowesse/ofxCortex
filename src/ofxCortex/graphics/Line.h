#pragma once

#include "ofxClipper.h"
#include "ofGraphics.h"
#include "ofMath.h"

namespace ofxCortex { namespace core { namespace graphics {

class Line {
public:
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
  
  // Helpers
  static ofPath polysToPath(const std::vector<ofPolyline> & polylines);
  
  static ofPolyline getLineSubsection(const ofPolyline & source, float start, float end);
  static ofPolyline getInterpolatedPolyline(const ofPolyline & source, const ofPolyline & target, float t);
  
  static void reverse(ofPolyline & source);
  static ofPolyline getReversed(const ofPolyline & source);
  
  static ofPolyline fromRectangle(const ofRectangle & rect);
  
private:
  Line() = default;
};

}}}
