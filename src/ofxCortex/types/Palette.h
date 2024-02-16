#pragma once

#include "ofColor.h"
#include "ofxTweenzor.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace types {

class Palette
{
public:
  enum class LookupMode {
    LINEAR,
    LINEAR_LOOP,
    COSINE,
    COSINE_LOOP
  };
  
public:
  Palette() {
    ofFboSettings settings;
    settings.width = 1000;
    settings.height = 1;
    settings.internalformat = GL_RGB;
    
    paletteFBO.allocate(settings);
    currentPaletteFBO.allocate(settings);
    nextPaletteFBO.allocate(settings);
    
    isDirty = true;
    
    ofAddListener(ofEvents().update, this, &Palette::update);
  }
  
  ~Palette() {
    ofRemoveListener(ofEvents().update, this, &Palette::update);
  }
  
  Palette(const Palette &other) : Palette() { this->setColors(other.colors); }
  Palette& operator=(const Palette &other)
  {
    if (this != &other) { this->setColors(other.colors); }
    return *this;
  }
  
  Palette(const std::vector<ofColor> & _colors) : Palette() { this->setColors(_colors); }
  Palette(std::initializer_list<ofColor> _colors) : Palette() { this->setColors(_colors); }
  
  operator std::vector<ofColor>() const { return colors; }
  operator const std::vector<ofColor>&() const { return colors; }
  ofColor & operator[](int i) { return colors[i]; }
  size_t size() const { return colors.size(); }
  
  // Adders
  void setColors(const std::vector<ofColor> & colors, float animationTime = 0.0f)
  {
    if (this->colors.size() == 0) currentPaletteMesh = ofxCortex::core::utils::Color::getGradientMesh(colors, paletteFBO.getWidth(), paletteFBO.getHeight());
    
    this->colors = colors;
    
    nextPaletteMesh = ofxCortex::core::utils::Color::getGradientMesh(colors, paletteFBO.getWidth(), paletteFBO.getHeight());
    
    isDirty = true;
    
    if (ofIsFloatEqual(animationTime, 0.0f)) onTransitionEnd(&transition);
    else Tweenzor::addCompleteListener(Tweenzor::add(&transition, 0.0f, 1.0f, 0.0f, animationTime, EASE_IN_OUT_CUBIC), this, &Palette::onTransitionEnd);
  }
  
  void setFromCoolors(const std::string & URL) { this->setColors(utils::Color::fromCoolors(URL)); }
  static Palette fromCoolors(const std::string & URL) { return Palette(utils::Color::fromCoolors(URL)); }
  
  // Getters
  const ofColor & getColor(unsigned int index) { return colors[CLAMP(index, 0, colors.size() - 1)]; }
  ofColor getColor(float t, LookupMode mode = LookupMode::LINEAR) {
    return paletteData.getColor(ofClamp(getLookupValue(t, mode), std::numeric_limits<float>::epsilon(), 1.0 - std::numeric_limits<float>::epsilon()) * paletteData.getWidth(), 0);
  };
  const std::vector<ofColor> & getColors() { return colors; }
  ofColor getRandomColor(float alpha = 1.0f) {
    ofColor c = utils::Array::randomInVector(colors);
    if (ofIsFloatEqual(alpha, 1.0f)) return c;
    
    c.a *= alpha;
    return c;
  }
  
  
  // Rendering
  void drawGradient(float x = 0.0, float y = 0.0, float w = 100.0, float h = 24.0) { paletteFBO.draw(x, y, w, h); }
  void drawColors(float x = 0.0, float y = 0.0, float w = 100.0, float h = 24.0)
  {
    float cellW = w / colors.size();
    float inset = 2;
    
    ofPushStyle();
    for (int i = 0; i < colors.size(); i++)
    {
      const ofColor & c = colors[i];
      
      float cX = cellW * i + x + inset;
      float cY = y + inset;
      float cW = cellW - inset * 2.0;
      float cH = h - inset * 2.0;
      
      ofSetColor(c);
      ofDrawRectRounded(cX, cY, cW, cH, 4);
    }
    ofPopStyle();
  }
  
  const ofTexture & getTexture() const { return paletteFBO.getTexture(); }
  
  
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
  
  ofEvent<void> onPaletteChanged;
  
protected:
  std::vector<ofColor> colors;
  
  void update(ofEventArgs & e)
  {
    if (this->isDirty) { this->updateFBOs(); }
  }
  
  void updateFBOs()
  {
    currentPaletteFBO.begin();
    {
      ofClear(0, 0, 0, 0);
      currentPaletteMesh.draw();
    }
    currentPaletteFBO.end();
    
    nextPaletteFBO.begin();
    {
      ofClear(0, 0, 0, 0);
      nextPaletteMesh.draw();
    }
    nextPaletteFBO.end();
    
    paletteFBO.begin();
    {
      ofClear(0, 0, 0, 0);
      
      ofSetColor(255, (1.0 - transition) * 255.0);
      currentPaletteFBO.draw(0, 0);
      
      ofSetColor(255, transition * 255.0);
      nextPaletteFBO.draw(0, 0);
    }
    paletteFBO.end();
    
    paletteFBO.readToPixels(paletteData);
  }
  
  ofFbo paletteFBO;
  ofFloatPixels paletteData;
  
  ofFbo currentPaletteFBO;
  ofMesh currentPaletteMesh;
  
  ofFbo nextPaletteFBO;
  ofMesh nextPaletteMesh;
  
  float transition { 0.0f };
  void onTransitionEnd(float * value)
  {
    updateFBOs();
    
    currentPaletteMesh = nextPaletteMesh;
    
    transition = 0.0f;
    isDirty = false;
    
    onPaletteChanged.notify();
  }
  
  float getLookupValue(float t, LookupMode mode)
  {
    switch (mode) {
      case LookupMode::LINEAR: return t;
      case LookupMode::LINEAR_LOOP: return 2.0 * abs(t - floor(t + 0.5));
      case LookupMode::COSINE: return cos(TWO_PI * 0.5 * t + PI) * 0.5 + 0.5;
      case LookupMode::COSINE_LOOP: return cos(TWO_PI * 1.0 * t + PI) * 0.5 + 0.5;
    }
  }
  
  bool isDirty { true };
  
};

}}}
