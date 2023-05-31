#pragma once

#include "ofMain.h"
#include "ofTrueTypeFont.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace graphics {

class Typography {
public:
  
  static void typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset = 0.0f, bool repeat = false, bool wrap = true);
  static void draw(const ofTrueTypeFont & font, const std::string & text, glm::vec2 pos, float fontSize, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  
protected:
};

}}}
