#pragma once

#include "ofRectangle.h"

namespace ofxCortex { namespace core { namespace graphics {

class Utils {
public:
  
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
        ofDrawGridPlane(100, 25);
        
        ofSetColor(255, 255 * alpha * 0.5);
        ofDrawGridPlane(10, 250);
      }
      ofPopStyle();
    }
    ofPopMatrix();
  }
};

}}}
