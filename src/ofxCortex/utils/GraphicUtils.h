#pragma once

#include "ofMesh.h"
#include "ofxCortex/types/Box.h"
#include "ofxCortex/utils/ShapingUtils.h"

namespace ofxCortex { namespace core { namespace utils {

namespace Graphics {

inline static glm::vec2 randomInRectangle(const ofRectangle & rect) { return { (int) ofRandom(rect.getLeft(), rect.getRight()), (int) ofRandom(rect.getTop(), rect.getBottom()) }; }

static void drawTexCoordRectangle(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f)
{
  static ofMesh mesh;
  
  if (!mesh.hasVertices())
  {
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.addVertex(glm::vec3(0, 0, 0));
    mesh.addTexCoord(glm::vec2(0, 0));
    
    mesh.addVertex(glm::vec3(1, 0, 0));
    mesh.addTexCoord(glm::vec2(1, 0));
    
    mesh.addVertex(glm::vec3(0, 1, 0));
    mesh.addTexCoord(glm::vec2(0, 1));
    
    mesh.addVertex(glm::vec3(1, 1, 0));
    mesh.addTexCoord(glm::vec2(1, 1));
    
    //    mesh.addTriangle(0, 1, 2);
    //    mesh.addTriangle(2, 3, 0);
  }
  
  ofPushMatrix();
  ofTranslate(x, y);
  ofScale(w, h);
  mesh.draw();
  ofPopMatrix();
}

static void drawTexCoordRectangle(const ofRectangle & rect) { drawTexCoordRectangle(rect.x, rect.y, rect.width, rect.height); }

static ofMesh getGradientMesh(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0)
{
  ofMesh mesh;
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
  
  int steps = w / 10.0;
  
  for (int i = 0; i < steps; i++)
  {
    float t = i / (float)(steps - 1);
    
    ofColor c = ofxCortex::core::utils::Shaping::interpolate(colors, t);
    
    float x = t * w;
    mesh.addVertex(glm::vec3(x, 0, 0));
    mesh.addColor(c);
    mesh.addVertex(glm::vec3(x, h, 0));
    mesh.addColor(c);
  }
  
  return mesh;
}

static ofMesh getGradientMeshVertical(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0)
{
  ofMesh mesh;
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
  
  int steps = h / 10.0;
  
  for (int i = 0; i < steps; i++)
  {
    float t = i / (float)(steps - 1);
    
    ofColor c = ofxCortex::core::utils::Shaping::interpolate(colors, t);
    
    float y = t * h;
    mesh.addVertex(glm::vec3(0, y, 0));
    mesh.addColor(c);
    mesh.addVertex(glm::vec3(w, y, 0));
    mesh.addColor(c);
  }
  
  return mesh;
}



}}}}
