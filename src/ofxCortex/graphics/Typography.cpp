#include "Typography.h"

namespace ofxCortex { namespace core { namespace graphics {



void Typography::draw(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  if (!font.isLoaded()) return;
  
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
  
  float scale = fontSize / font.getSize();
  static auto xBB = font.getStringBoundingBox("X", 0, 0);
  auto BB = font.getStringBoundingBox(text, 0, 0);
  
  float offsetX = -BB.width * horzAlignMultipliers[horizontalAlign];
  float offsetY = -xBB.height * vertAlignMultipliers[verticalAlign] + xBB.height;
  
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
  draw(font.getInternalFont(), text, pos, font.getFontSize(), horizontalAlign, verticalAlign);
  
//  if (!font.isLoaded()) return;
//  
//  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
//  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
//  
//  auto BB = font.getInternalFont().getStringBoundingBox(text, 0, 0);
//  
//  float offsetX = -BB.width * horzAlignMultipliers[horizontalAlign];
//  float offsetY = -BB.height * vertAlignMultipliers[verticalAlign] + font.heightX;
//  
//  ofPushMatrix();
//  ofTranslate(pos);
//  ofScale(font.getScale());
//  {
//    font.getInternalFont().drawStringAsShapes(text, offsetX, offsetY);
//  }
//  ofPopMatrix();
}

std::vector<glm::vec2> Typography::getStringAsPoints(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  std::vector<glm::vec2> points;
  
  if (!font.isLoaded()) return points;
  
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
  
  float scale = fontSize / font.getSize();
  static auto xBB = font.getStringBoundingBox("X", 0, 0);
  auto BB = font.getStringBoundingBox(text, 0, 0);
  
  float offsetX = -BB.width * horzAlignMultipliers[horizontalAlign];
  float offsetY = -BB.height * vertAlignMultipliers[verticalAlign] + xBB.height;
  
  auto paths = font.getStringAsPoints(text);
  for (auto & path : paths)
  {
    path.scale(scale, scale);
    path.translate(pos);
    
    path.setPolyWindingMode(OF_POLY_WINDING_ODD);
  }
  
  for (int i = 0; i < paths.size(); i++) 
  {
    const auto & path = paths[i];
    
    for (auto & outline : path.getOutline())
    {
      for (auto & p : outline) points.push_back(glm::vec2(p));
    }
  }
  
  return points;
}

std::vector<glm::vec2> Typography::getStringAsPoints(const Font & font, const std::string & text, const glm::vec2 & pos, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  return getStringAsPoints(font.getInternalFont(), text, pos, font.getFontSize(), horizontalAlign, verticalAlign);
}

std::vector<ofPolyline> Typography::getStringAsLines(const ofTrueTypeFont & font, const std::string & text, const glm::vec2 & pos, float fontSize, ofAlignHorz horizontalAlign, ofAlignVert verticalAlign)
{
  std::vector<ofPolyline> output;
  
  if (!font.isLoaded()) return output;
  
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  static map<ofAlignVert, float> vertAlignMultipliers = { { OF_ALIGN_VERT_TOP, 0.0f }, { OF_ALIGN_VERT_CENTER, 0.5 }, { OF_ALIGN_VERT_BOTTOM, 1.0f }};
  
  float scale = fontSize / font.getSize();
  static auto xBB = font.getStringBoundingBox("X", 0, 0);
  auto BB = font.getStringBoundingBox(text, 0, 0);
  
  glm::vec2 offset = { -BB.width * horzAlignMultipliers[horizontalAlign], -BB.height * vertAlignMultipliers[verticalAlign] + xBB.height };
  
  auto paths = font.getStringAsPoints(text);
  for (auto & path : paths)
  {
    path.scale(scale, scale);
    path.translate(pos + offset * scale);
    
    path.setPolyWindingMode(OF_POLY_WINDING_ODD);
  }
  
  for (int i = 0; i < paths.size(); i++)
  {
    const auto & path = paths[i];
    
    for (auto & outline : path.getOutline())
    {
      output.push_back(outline);
    }
  }
  
  return output;
}


void Typography::typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, const Typography::TypeOnPathSettings & settings)
{
  typeOnPath(font, text, line, settings.fontSize, settings.offset, settings.horizontalAlign, settings.spacing, settings.repeat, settings.wrap, settings.flipX, settings.flipY);
}

void Typography::typeOnPath(const ofTrueTypeFont & font, const std::string & text, const ofPolyline & line, float fontSize, float offset, ofAlignHorz horizontalAlign, float spacing, bool repeat, bool wrap, bool flipX, bool flipY)
{
  static map<ofAlignHorz, float> horzAlignMultipliers = { { OF_ALIGN_HORZ_LEFT, 0.0f }, { OF_ALIGN_HORZ_CENTER, 0.5 }, { OF_ALIGN_HORZ_RIGHT, 1.0f }};
  
  float scale = fontSize / font.getSize();
  auto xBB = font.getStringBoundingBox("X", 0, 0);
  
  int directionX = (flipX) ? -1 : 1;
  int directionY = (flipY) ? -1 : 1;
  
  float totalWidth = ((xBB.width * scale) * (1.0 + spacing)) * text.size();
  
  offset = ofWrap(offset - (totalWidth * horzAlignMultipliers[horizontalAlign]), 0, line.getPerimeter());
  
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
    
    lineX += (xBB.width * scale) * (1.0 + spacing) * directionX;
    letterIndex++;
  }
}

}}}
