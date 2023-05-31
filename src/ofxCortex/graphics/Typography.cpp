#include "Typography.h"

namespace ofxCortex { namespace core { namespace graphics {

void Typography::typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset, bool repeat, bool wrap)
{
  float scale = fontSize / font.getSize();
  auto xBB = font.getStringBoundingBox("X", 0, 0);
  
  offset = ofWrap(offset, 0, line.getPerimeter());
  
  int letterIndex = 0;
  for (float lineX = 0; lineX < line.getPerimeter() - xBB.width * scale;)
  {
    if (letterIndex > text.size() - 1 && !repeat) break;
    if (!wrap && lineX + offset > line.getPerimeter()) break;
    
    string letter = ofToString(text[letterIndex % text.size()]);
    auto BB = font.getStringBoundingBox(letter, 0, 0);
    auto scaledBB = BB; scaledBB.scale(scale);
    
    float actualX = lineX + offset;
    float wrappedX = ofWrap(lineX + offset, 0, line.getPerimeter());
    
    float index = line.getIndexAtLength(wrappedX);
    
    
    glm::vec3 pos = line.getPointAtIndexInterpolated(index);
    float rot = utils::Vector::toAngle(line.getNormalAtIndexInterpolated(index + 0.000001)) + HALF_PI;
    
    ofPushMatrix();
    ofTranslate(pos);
    ofRotateRad(rot);
    ofScale(scale);
    
    font.drawString(letter, -xBB.width * 0.5, xBB.height * 0.5);
    ofPopMatrix();
    
    lineX += scaledBB.width;
    letterIndex++;
  }
}

void Typography::draw(const ofTrueTypeFont & font, const std::string & text, glm::vec2 pos, float fontSize, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
  
  float scale = fontSize / font.getSize();
  auto xBB = font.getStringBoundingBox("X", 0, 0);
  auto BB = font.getStringBoundingBox(text, 0, 0);
  
  float offsetX = -BB.width * horzAlignMultipliers[horizontalAlign];
  float offsetY = -BB.height * vertAlignMultipliers[verticalAlign] + xBB.height;
  
  ofPushMatrix();
  ofTranslate(pos);
  ofScale(scale);
  {
    font.drawStringAsShapes(text, offsetX, offsetY);
  }
  ofPopMatrix();
}


}}}
