#pragma once

#include "ofxClipper.h"

namespace ofxCortex { namespace graphics {

class Line {
public:
  static ofPath getLinePath(const ofPolyline & source, float thickness, ClipperLib::JoinType jointype = ClipperLib::jtRound, ClipperLib::EndType endtype = ClipperLib::etOpenRound);
  static void drawPolyline(const ofPolyline & source, float thickness, ClipperLib::JoinType jointype = ClipperLib::jtRound, ClipperLib::EndType endtype = ClipperLib::etOpenRound);

  static void catmullRom(ofPolyline & target, const ofPolyline & source, int iterations = 10);
  static void chaikin(ofPolyline & target, const ofPolyline & source, int iterations = 4, float tension = 0.5f);
  
  static ofPath polysToPath(const std::vector<ofPolyline> & polylines);
  
private:
  Line() = default;
};

}}
