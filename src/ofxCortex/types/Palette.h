#pragma once

#include "ofColor.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace types {

class Palette
{
public:
  ofEvent<void> onPaletteChanged;
  
  Palette(std::vector<ofColor> _colors) : colors(_colors) {}
  
  Palette(const Palette &other) : colors(other.colors)  { }
  
  Palette& operator= (const Palette &other)
  {
    if (this != &other)
    {
      this->colors = other.colors;
      onPaletteChanged.notify();
    }
    return *this;
  }
  
  // Adders
  void addColor(ofColor color) { colors.push_back(color); onPaletteChanged.notify(); }
  void addColors(std::vector<ofColor> _colors) { colors.insert(colors.end(), _colors.begin(), _colors.end()); onPaletteChanged.notify(); }
  
  static Palette fromCoolors(const std::string & URL) { return Palette(utils::Color::fromCoolors(URL)); }
  
  // Getters
  ofColor getColor(unsigned int index) { return colors[CLAMP(index, 0, colors.size() - 1)]; }
  ofColor getRandomColor() { return utils::Array::randomInVector(colors); }
  
  
  // Serialization
  friend std::ostream& operator<<(std::ostream& os, const Palette & palette)
  {
    for (auto color : palette.colors)
    {
      os << color.getHex() << ",";
    }
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, Palette & palette)
  {
    std::string incoming(std::istreambuf_iterator<char>(is), {});
    
    std::vector<string> hexValues = ofSplitString(incoming, ",");
    
    std::vector<ofColor> colors;
    for (auto & hexString : hexValues)
    {
      unsigned int hexValue;
      std::istringstream iss(hexString);
      iss >> std::hex >> hexValue;
      
      int r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
      int g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
      int b = ((hexValue) & 0xFF);
      
      colors.push_back(ofColor(r, g, b));
    }
    
    palette = Palette(colors);
    
    return is;
  }
  
protected:
  std::vector<ofColor> colors;
  
};

}}}
