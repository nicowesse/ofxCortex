#pragma once

#include "ofMain.h"
#include "ofTrueTypeFont.h"
#include "ofxCortex/utils/VectorUtils.h"

namespace ofxCortex { namespace core { namespace graphics {

class Typography {
public:
  struct Font {
    friend class Typography;
    
    Font() {
      font = make_shared<ofTrueTypeFont>();
    }
    
    Font(const Font &other)  {
      font = make_shared<ofTrueTypeFont>(other.getInternalFont());
      fontSize = other.fontSize;
      this->recalculateProperties();
    }
    
    Font& operator= (const ofTrueTypeFont &font)
    {
      this->font = std::make_shared<ofTrueTypeFont>(font);
      
      return *this;
    }
    
    operator const ofTrueTypeFont&() const { return *font; }
    
    void load(const ofTrueTypeFontSettings & settings)
    {
      font->load(settings);
      this->recalculateProperties();
    }
    
    void load(const std::string & path, float fontSize = 200.0f)
    {
      ofTrueTypeFontSettings settings(path, fontSize);
      settings.antialiased = true;
      settings.contours = true;
      settings.addRanges(ofAlphabet::Latin);
      
      font->load(settings);
      this->recalculateProperties();
    }
    
    bool isLoaded() const { return font->isLoaded(); }
    
    float getFontSize() const { return fontSize; }
    void setFontSize(float size) { this->fontSize = size; this->recalculateProperties(); }
    
    float getXHeight() const { return heightX; }
    float getAscenderHeight() const { return ascenderHeight; }
    float getDescenderHeight() const { return descenderHeight; }
    
    ofRectangle getBoundingBox(const std::string & str) const { return ofRectangle(0, 0, font->stringWidth(str) * getScale(), font->stringHeight(str) * getScale()); }
    ofRectangle getBoundingBox(const std::string & str, float fontSize) const { return ofRectangle(0, 0, font->stringWidth(str) * (fontSize / font->getSize()), font->stringHeight(str)  * (fontSize / font->getSize())); }
    
    float getLineHeight() const { return font->getLineHeight() * getScale(); }
    float getSpaceSize() const { return spaceSize; }
    
    const ofTrueTypeFont & getInternalFont() const { return *font; }
    
  protected:
    shared_ptr<ofTrueTypeFont> font;
    float fontSize { 24 };
    float getScale() const { return fontSize / font->getSize(); }
    
    // Properties
    void recalculateProperties()
    {
      heightX = getBoundingBox("X").height;
      ascenderHeight = getBoundingBox("f").height;
      descenderHeight = getBoundingBox("g").height;
      spaceSize = getBoundingBox("p").width * font->getSpaceSize();
    }
    
    float heightX;
    float ascenderHeight;
    float descenderHeight;
    float spaceSize;
  };
  
#pragma mark - Font Drawing
  struct FontSettings {
    FontSettings(float size) : fontSize(size) {};
    
    float fontSize;
    ofAlignHorz alignHorz { OF_ALIGN_HORZ_LEFT };
    ofAlignVert alignVert { OF_ALIGN_VERT_BOTTOM };
    float spacing { 0.0f };
    
    float getVerticalAlignmentMultiplier() {
      static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
      
      return vertAlignMultipliers[alignVert];
    }
    
    float getHorizontalAlignmentMultiplier() {
      static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
      
      return horzAlignMultipliers[alignHorz];
    }
  };
  
  static void draw(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  static void draw(const Font & font, const std::string & text, const glm::vec2 & pos, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  
  static std::vector<glm::vec2> getStringAsPoints(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  static std::vector<glm::vec2> getStringAsPoints(const Font & font, const std::string & text, const glm::vec2 & pos, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  
  static std::vector<ofPolyline> getStringAsLines(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign = OF_ALIGN_HORZ_LEFT, ofAlignVert verticalAlign = OF_ALIGN_VERT_TOP);
  
#pragma mark - Type-on-Path
  struct TypeOnPathSettings : public FontSettings {
    TypeOnPathSettings(float size, float offset = 0.0f, bool repeat = false, bool wrap = false, bool flipX = false, bool flipY = false)
    : FontSettings(size), offset(offset), repeat(repeat), wrap(wrap), flipX(flipX), flipY(flipY) {};
    
    float offset;
    bool repeat { false };
    bool wrap { false };
    bool flipX { false };
    bool flipY { false };
  };
  
  static void typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset = 0.0f, float spacing = 1.0f, bool repeat = false, bool wrap = true, bool flipX = false, bool flipY = false);
  static void typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, const TypeOnPathSettings & settings);
protected:
};

}}}
