#include "Typography.h"

namespace ofxCortex { namespace core { namespace graphics {



void Typography::draw(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  if (!font.isLoaded()) return;
  
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


void Typography::draw(const Font & font, const std::string & text, const glm::vec2 & pos, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  if (!font.isLoaded()) return;
  
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
  
  auto BB = font.getInternalFont().getStringBoundingBox(text, 0, 0);
  
  float offsetX = -BB.width * horzAlignMultipliers[horizontalAlign];
  float offsetY = -BB.height * vertAlignMultipliers[verticalAlign] + font.heightX;
  
  ofPushMatrix();
  ofTranslate(pos);
  ofScale(font.getScale());
  {
    font.getInternalFont().drawStringAsShapes(text, offsetX, offsetY);
  }
  ofPopMatrix();
}


void Typography::typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, const Typography::TypeOnPathSettings & settings)
{
//  float scale = settings.fontSize / font.getSize();
//  auto xBB = font.getStringBoundingBox("X", 0, 0);
//  
//  int direction = (settings.flip) ? -1 : 1;
//  
//  float offset = ofWrap(settings.offset, 0, line.getPerimeter());
//  
//  int letterIndex = 0;
//  for (float lineX = 0; abs(lineX) < line.getPerimeter() - xBB.width * scale;)
//  {
//    if (letterIndex > text.size() - 1 && !settings.repeat) break;
//    if (lineX + offset > line.getPerimeter() && !settings.wrap) break;
//    
//    string letter = ofToString(text[letterIndex % text.size()]);
//    bool isSpace = text[letterIndex % text.size()] == ' ';
//    
//    auto BB = (isSpace) ? font.getStringBoundingBox("p", 0, 0) : font.getStringBoundingBox(letter, 0, 0);
//    auto scaledBB = BB; scaledBB.scale(scale);
//    
//    float actualX = offset;
//    float wrappedX = ofWrap(actualX, 0, line.getPerimeter());
//    
//    float index = line.getIndexAtLength(wrappedX);
//    
//    glm::vec3 pos = line.getPointAtIndexInterpolated(index);
//    float rot = utils::Vector::toRadians(line.getNormalAtIndexInterpolated(index + 0.000001)) + HALF_PI;
//    
//    ofPushMatrix();
//    ofTranslate(pos);
//    ofRotateRad(rot);
//    ofScale(scale * direction);
//    
//    font.drawStringAsShapes(letter, 0, xBB.height * 0.5);
//    ofPopMatrix();
//    
//    lineX += scaledBB.width * settings.spacing * direction;
//    letterIndex++;
//  }
  
  typeOnPath(font, text, line, settings.fontSize, settings.offset, settings.spacing, settings.repeat, settings.wrap, settings.flipX, settings.flipY);
}

void Typography::typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset, float spacing, bool repeat, bool wrap, bool flipX, bool flipY)
{
  float scale = fontSize / font.getSize();
  auto xBB = font.getStringBoundingBox("X", 0, 0);
  
  int directionX = (flipX) ? -1 : 1;
  int directionY = (flipY) ? -1 : 1;
  
  offset = ofWrap(offset, 0, line.getPerimeter());
  
  int letterIndex = 0;
  for (float lineX = 0; abs(lineX) < line.getPerimeter() - xBB.width * scale;)
  {
    if (letterIndex > text.size() - 1 && !repeat) break;
    if (!wrap && lineX + offset > line.getPerimeter()) break;
    
    string letter = ofToString(text[letterIndex % text.size()]);
    bool isSpace = text[letterIndex % text.size()] == ' ';
    
    auto BB = (isSpace) ? font.getStringBoundingBox("p", 0, 0) : font.getStringBoundingBox(letter, 0, 0);
    auto scaledBB = BB; scaledBB.scale(scale);
    
    float actualX = lineX + offset;
    float wrappedX = ofWrap(actualX, 0, line.getPerimeter());
    
    float index = line.getIndexAtLength(wrappedX);
    
    glm::vec3 pos = line.getPointAtIndexInterpolated(index);
    float rot = utils::Vector::toRadians(line.getNormalAtIndexInterpolated(index + 0.000001)) + HALF_PI;
    
    ofPushMatrix();
    ofTranslate(pos);
    ofRotateRad(rot);
    ofScale(scale * directionX, scale * directionY);
    
    font.drawStringAsShapes(letter, -xBB.width * 0.5, xBB.height * 0.5);
    ofPopMatrix();
    
    lineX += scaledBB.width * (1.0 + spacing) * directionX;
    letterIndex++;
  }
}

}}}
