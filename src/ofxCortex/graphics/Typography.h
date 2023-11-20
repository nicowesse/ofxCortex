#pragma once

#include "ofMain.h"
#include "ofTrueTypeFont.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace graphics {

class Typography {
public:
  struct Font {
    friend class Typography;
    
    Font() : heightX(xBB.height) {
      font = make_shared<ofTrueTypeFont>();
      
    }
    
    void load(const ofTrueTypeFontSettings & settings)
    {
      font->load(settings);
      xBB = getBoundingBox("X");
      spaceSize = getBoundingBox("p").width * font->getSpaceSize();
    }
    
    bool isLoaded() const { return font->isLoaded(); }
    
    float getFontSize() const { return fontSize; }
    void setFontSize(float size) { this->fontSize = size; }
    
    float getXHeight() const { return heightX; }
    float getAscenderHeight() const { return getBoundingBox("f").height; }
    float getDescenderHeight() const { return getBoundingBox("g").height; }
    
    ofRectangle getBoundingBox(const std::string & str) const { return ofRectangle(0, 0, font->stringWidth(str) * getScale(), font->stringHeight(str) * getScale()); }
    
    float getLineHeight() const { return font->getLineHeight() * getScale(); }
    float getSpaceSize() const { return getBoundingBox("p").width * font->getSpaceSize(); }
    
    const ofTrueTypeFont & getInternalFont() const { return *font; }
    
  protected:
    shared_ptr<ofTrueTypeFont> font;
    float fontSize { 24 };
    float getScale() const { return fontSize / font->getSize(); }
    ofRectangle xBB;
    const float & heightX;
    float spaceSize;
  };
  
  static void typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset = 0.0f, bool repeat = false, bool wrap = true);
  static void draw(const ofTrueTypeFont & font, const std::string & text, glm::vec2 pos, float fontSize, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  static void draw(const std::string & text, glm::vec2 pos, const Font & settings, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
protected:
};

}}}
