#pragma once

#include "of3dGraphics.h"

namespace ofxCortex { namespace core { namespace types {

class Box {
public:
  Box() : x(position.x), y(position.y), z(position.z) { set(0, 0, 0, 0, 0, 0); }
  Box(const glm::vec3 & p, float w, float h, float d) : Box() { set(p, w, h, d); }
  Box(float px, float py, float pz, float w, float h, float d) : Box() { set(px, py, pz, w, h, d); }
  Box(const Box & box) : Box() { set(box.x, box.y, box.z, box.width, box.height, box.depth); }
  
  virtual ~Box() {};
  
  void set(float px, float py, float pz, float w, float h, float d)
  {
    x = px;
    y = py;
    z = pz;
    width = w;
    height = h;
    depth = d;
    
    cout << "Box: " << x << ", " << y << ", " << z << " " << width << ", " << height << ", " << depth << endl;
  }
  
  void set(const glm::vec3 & p, float w, float h, float d) { set(p.x, p.y, p.z, w, h, d); }
  
  float getLeft() const { return x; }
  float getRight() const { return x + width; }
  float getTop() const { return y; }
  float getBottom() const { return y + height; }
  float getFront() const { return z; }
  float getBack() const { return z + depth; }
  
  void draw()
  {
    ofPushMatrix();
    
  //  ofTranslate(width * 0.5, height * 0.5, depth * 0.5);
    
    ofDrawBox(position + glm::vec3(width * 0.5, height * 0.5, depth * 0.5), width, height, depth);
    
    ofPopMatrix();
  };
  
  bool inside(const glm::vec3 & p) const
  {
    return (p.x >= getLeft() && p.x <= getRight()) && (p.y >= getTop() && p.y <= getBottom()) && (p.z >= getFront() && p.z <= getBack());
  }
  
  // Properties
  glm::vec3 position{};
  float& x;
  float& y;
  float& z;
  
  float width;
  float height;
  float depth;
};

}}}


//ostream& operator<<(ostream& os, const ofxCortex::core::types::Box& box) {
//  os << box.position << ", " << box.width << ", " << box.height << ", " << box.depth;
//  return os;
//}

////----------------------------------------------------------
//istream& operator>>(istream& is, ofxCortex::core::types::Box& box) {
//  is >> box.position;
//  is.ignore(2);
//  is >> box.width;
//  is.ignore(2);
//  is >> box.height;
//  is.ignore(2);
//  is >> box.depth;
//  return is;
//}
