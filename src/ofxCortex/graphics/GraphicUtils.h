#pragma once

#include "ofRectangle.h"
#include "ofGraphics.h"
#include "of3dGraphics.h"
#include "of3dUtils.h"

namespace ofxCortex { namespace core { namespace graphics {

class Utils {
public:
  static void drawAxis(const glm::vec3 & position = glm::vec3(0), float scale = 10.0f)
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
  
  static void drawGrid(const ofRectangle & bounds, int columns, int rows, ofColor lineColor = ofColor(255), bool drawCenters = false)
  {
    const float cellW = bounds.width / columns;
    const float cellH = bounds.height / rows;
    
    ofPushStyle();
    ofSetColor(lineColor, 64);
    for (int row = 0; row <= rows; row++)
    {
      float y = row * cellH + bounds.y;
      ofDrawLine(bounds.getLeft(), y, bounds.getRight(), y);
      
      for (int col = 0; col <= columns; col++)
      {
        float x = col * cellW + bounds.x;
        
        if (row == 0) ofDrawLine(x, bounds.getTop(), x, bounds.getBottom());
      }
    }
    
    float offset = 6;
    ofSetColor(lineColor, 128);
    for (int row = 0; row <= rows; row++)
    {
      for (int col = 0; col <= columns; col++)
      {
        float x = col * cellW + bounds.x;
        float y = row * cellH + bounds.y;
        
        ofDrawLine(x - offset, y, x + offset, y);
        ofDrawLine(x, y - offset, x, y + offset);
        
        if (drawCenters) ofDrawCircle(x + cellW * 0.5, y + cellH * 0.5, 1);
      }
    }
    ofPopStyle();
  }
  
  static void drawPlane(float alpha = 0.1f)
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
};

}}}
