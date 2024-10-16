#pragma once

#include "ofGraphics.h"
#include "of3dGraphics.h"
#include "of3dUtils.h"

#include "Helpers.h"
#include "VectorUtils.h"

namespace ofxCortex { namespace core { namespace utils {

namespace Debug {

enum class Cap {
  NONE, ARROW, LINE, DOT
};

// 2D
inline static void drawCap(const glm::vec2 & P, const glm::vec2 & D, Cap cap, float size = 8.0)
{
  if (cap == Cap::NONE) return;
  
  ofPushMatrix();
  {
    ofTranslate(P);
    ofRotateRad(Vector::toRadians(D) + HALF_PI);
    ofPushStyle();
    {
      ofFill();
      if (cap == Cap::ARROW)
      {
        ofBeginShape();
        {
          ofVertex(0, 0);
          ofVertex(+size * 0.5, size * 0.866);
          ofVertex(-size * 0.5, size * 0.866);
        }
        ofEndShape(OF_CLOSE);
      }
      else if (cap == Cap::LINE) ofDrawLine(-size * 0.75, 0, +size * 0.75, 0);
      else if (cap == Cap::DOT) ofDrawCircle(0, 0, 3);
    }
    ofPopStyle();
  }
  ofPopMatrix();
}

inline static void drawLine(const ofPolyline & source, float dash = 0.0, float gap = 4.0, Cap startCap = Cap::NONE, Cap endCap = Cap::NONE)
{
  if (source.size() < 2) return;
  
  if (dash > 0.0)
  {
    float length = source.getPerimeter();
    float pixelStep = 1.0 / length;
    float dashGapTotal = dash + gap;
    
    int steps = length / (dash + gap);
    
    const auto & viewport = ofGetCurrentViewport();
    
    for (int step = 0; step <= steps; step++)
    {
      glm::vec2 from = source.getPointAtPercent(step * pixelStep * dashGapTotal);
      glm::vec2 to = source.getPointAtPercent(step * pixelStep * dashGapTotal + pixelStep * dash);
        
      if (!viewport.inside(from) || !viewport.inside(to)) continue;
      
      ofDrawLine(from, to);
    }
  }
  else source.draw();
  
  const glm::vec2 & start = source[0];
  glm::vec2 startDir = glm::normalize(start - source[1]);
  
  drawCap(start, startDir, startCap);
  
  const glm::vec2 & end = source[source.size() - 1];
  glm::vec2 endDir = glm::normalize(end - source[source.size() - 2]);
  
  drawCap(end, endDir, endCap);
}

inline static void drawLine(const glm::vec2 & a, const glm::vec2 & b, float dash = 0.0, float gap = 4, Cap startCap = Cap::NONE, Cap endCap = Cap::NONE)
{
  ofPolyline line; line.addVertex(a.x, a.y); line.addVertex(b.x, b.y);
  drawLine(line, dash, gap, startCap, endCap);
}

inline static void drawDashedLine(const glm::vec2 & a, const glm::vec2 & b, float dash = 4.0, float gap = 4, Cap startCap = Cap::NONE, Cap endCap = Cap::NONE)
{
  drawLine(a, b, dash, gap, startCap, endCap);
}

inline static void drawArrow(const glm::vec2 & a, const glm::vec2 & b, bool dashed = false)
{
  drawLine(a, b, 4.0 * dashed, 4.0, Cap::NONE, Cap::ARROW);
}

inline static void drawLabeledLine(const glm::vec2 & a, const glm::vec2 & b, const std::string & label, bool dashed = false, Cap startCap = Cap::NONE, Cap endCap = Cap::ARROW)
{
  glm::vec2 dir { b - a };
  glm::vec2 mid = glm::mix(a, b, 0.5);
  
  drawLine(a, b, 4 * dashed, 4, startCap, endCap);
  ofDrawBitmapString(label, mid.x + label.length() * 8 * -0.5 - 1, mid.y + 4);
}

inline static void drawCircle(const glm::vec2 & P, float r, float dash = 0.0)
{
  ofPolyline circle; circle.arc(glm::vec3(P, 0), r, r, 0, 360, true, 180);
  
  ofPushStyle();
  ofNoFill();
  drawLine(circle, dash, dash, utils::Debug::Cap::NONE, utils::Debug::Cap::NONE);
  ofPopStyle();
}

inline static void drawAngle(const glm::vec2 & C, const glm::vec2 & A, const glm::vec2 & B, float arcRadius = 60)
{
  glm::vec2 CtoA = glm::normalize(A - C);
  float angleA = utils::Vector::toDegrees(CtoA);
  
  glm::vec2 CtoB = glm::normalize(B - C);
  float angleB = utils::Vector::toDegrees(CtoB);
  
  bool clockwise = true;
  float actualAngle = angleB - angleA;
  
  if (angleA > angleB) 
  {
    clockwise = !clockwise;
    actualAngle = abs(actualAngle);
  }
  
  std::string label = ofToString(actualAngle, 1);
  
  float aR = std::max(65.0f, arcRadius);
  
  glm::vec2 labelPoint = C + slerp(CtoA, CtoB, 0.5) * aR * 0.66;
  
  drawLine(C, A, 4, 4, utils::Debug::Cap::NONE, utils::Debug::Cap::NONE);
  drawLine(C, B, 4, 4, utils::Debug::Cap::NONE, utils::Debug::Cap::NONE);
  
  ofPath shape; shape.setFillColor(ofColor(ofGetStyle().color, 32)); shape.setStrokeColor(ofGetStyle().color); shape.setStrokeWidth(1.0); shape.setCircleResolution(std::max(aR / 3.0f, 90.f)); shape.moveTo(C); shape.arc(C, aR, aR, angleA, angleB, clockwise); shape.close();
  
  if (ofIsFloatEqual(actualAngle, 90.0f))
  {
    shape.moveTo(C); shape.lineTo(C + CtoA * 12); shape.lineTo(C + CtoA * 12 + CtoB * 12); shape.lineTo(C + CtoB * 12); shape.close();
  }

  shape.draw();
  
  ofDrawBitmapString(label, labelPoint + glm::vec2(-16, 5));
}

inline static void drawPoint(const glm::vec2 & p, const std::string & label = "", const glm::vec2 & labelOffset = glm::vec2(0))
{
  ofPushStyle();
  ofFill();
  ofDrawCircle(p.x, p.y, 3);
  ofPopStyle();
  
  std::stringstream labelStream;
  if (label == " ") labelStream << ofToString(p.x, 1) << ", " << ofToString(p.y, 1);
  else labelStream << label;
  
  ofDrawBitmapString(labelStream.str(), p.x + 8 + labelOffset.x, p.y + 4 + labelOffset.y);
}

inline static void drawPolyline(const ofPolyline & source)
{
  if (source.size() < 2) return;
  
  for (int i = 0; i < source.size() - !source.isClosed(); i++)
  {
    const auto & p = source[(i + 0) % source.size()];
    const auto & n = source[(i + 1) % source.size()];
    
    drawDashedLine(p, n);
    drawPoint(p);
  }
  
  if (!source.isClosed()) drawPoint(source.getVertices().back());
}

// 3D
inline static void drawAxis(const glm::vec3 & position = glm::vec3(0), float scale = 10.0f)
{
  ofPushMatrix();
  {
    ofTranslate(position);
    ofScale(scale);
    
    ofPushStyle();
    {
      ofFill();
      
      ofSetColor(ofColor::tomato);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), 0.05);
      ofDrawBitmapString("X", 1.1, -0.025, -0.025);
      
      ofSetColor(ofColor::springGreen);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 0.05);
      ofDrawBitmapString("Y", 0, 1.1, 0);
      
      ofSetColor(ofColor::dodgerBlue);
      ofDrawArrow(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), 0.05);
      ofDrawBitmapString("Z", -0.025, -0.025, 1.1);
      
      ofSetColor(ofColor::black);
      ofDrawSphere(0.05);
    }
    ofPopStyle();
  }
  ofPopMatrix();
}

inline static void drawGrid(const glm::vec2 & position, float cellW, float cellH, int columns, int rows, ofColor lineColor = ofColor(255), bool drawCenters = false)
{
  ofPushStyle();
  ofSetColor(lineColor, 64);
  for (int row = 0; row <= rows; row++)
  {
    float y = row * cellH + position.y;
    ofDrawLine(position.x, y, position.x + cellW * columns, y);
    
    for (int col = 0; col <= columns; col++)
    {
      float x = col * cellW + position.x;
      
      if (row == 0) ofDrawLine(x, position.y, x, position.y + cellH * rows);
    }
  }
  
  float offset = 6;
  ofSetColor(lineColor, 128);
  for (int row = 0; row <= rows; row++)
  {
    for (int col = 0; col <= columns; col++)
    {
      float x = col * cellW + position.x;
      float y = row * cellH + position.y;
      
      ofDrawLine(x - offset, y, x + offset, y);
      ofDrawLine(x, y - offset, x, y + offset);
      
      if (drawCenters) ofDrawCircle(x + cellW * 0.5, y + cellH * 0.5, 1);
    }
  }
  ofPopStyle();
}

inline static void drawGrid(const ofRectangle & bounds, int columns, int rows, ofColor lineColor = ofColor(255), bool drawCenters = false)
{
  drawGrid(bounds.position, bounds.width / columns, bounds.height / rows, columns, rows, lineColor, drawCenters);
}

inline static void drawPlane(float alpha = 0.1f)
{
  ofPushMatrix();
  {
    ofRotateZDeg(90);
    ofPushStyle();
    {
      ofSetColor(255, 255 * alpha);
      ofDrawGridPlane(1000, 25);
      
      ofSetColor(255, 255 * alpha * 0.5);
      ofDrawGridPlane(100, 250);
    }
    ofPopStyle();
  }
  ofPopMatrix();
  
  ofPushStyle();
  {
    ofSetColor(255, 255 * alpha * 1.5);
    ofDrawLine(-100 * 25, 1, 0, 100 * 25, 1, 0);
    ofDrawLine(0, 1, -100 * 25, 0, 1, 100 * 25);
  }
  ofPopStyle();
}

}}}}
