#pragma once

#include "ofColor.h"
#include "ofxTweenzor.h"
#include "ofxCortex/utils/Helpers.h"
#include "ofxCortex/utils/GraphicUtils.h"

namespace ofxCortex { namespace core { namespace types {

#define STRINGIFY(x) #x

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
  Palette(const std::string & string) : Palette() { this->setFromCoolors(string); }
  
  operator std::vector<ofColor>() const { return colors; }
  operator const std::vector<ofColor>&() const { return colors; }
  ofColor & operator[](int i) { return colors[i]; }
  size_t size() const { return colors.size(); }
  
  // Adders
  void setColors(const std::vector<ofColor> & colors, float animationTime = 0.0f)
  {
    if (this->colors.size() == 0) currentPaletteMesh = ofxCortex::core::utils::Graphics::getGradientMesh(colors, paletteFBO.getWidth(), paletteFBO.getHeight());
    
    this->colors = colors;
    
    nextPaletteMesh = ofxCortex::core::utils::Graphics::getGradientMesh(colors, paletteFBO.getWidth(), paletteFBO.getHeight());
    
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
  
  static Palette blackToWhite() { return Palette({ ofColor::black, ofColor::white }); }
  static Palette whiteToBlack() { return Palette({ ofColor::white, ofColor::black }); }
  static Palette rainbow(int steps = 6) { auto colors = ofxCortex::core::utils::Array::constructVector<ofColor>(steps, [](int i, float t) { return ofColor::fromHsb(t * 255.0, 255.0, 255.0); }); return Palette(colors); }
  static Palette dayCycle() { return Palette({ ofColor::fromHex(0x040b3c), ofColor::fromHex(0x012459), ofColor::fromHex(0x003972), ofColor::fromHex(0x003972), ofColor::fromHex(0x004372), ofColor::fromHex(0x004372), ofColor::fromHex(0x016792), ofColor::fromHex(0x07729f), ofColor::fromHex(0x12a1c0), ofColor::fromHex(0x74d4cc), ofColor::fromHex(0xefeebc), ofColor::fromHex(0xfee154), ofColor::fromHex(0xfdc352), ofColor::fromHex(0xffac6f), ofColor::fromHex(0xfda65a), ofColor::fromHex(0xfd9e58), ofColor::fromHex(0xf18448), ofColor::fromHex(0xf06b7e), ofColor::fromHex(0xca5a92), ofColor::fromHex(0x5b2c83), ofColor::fromHex(0x371a79), ofColor::fromHex(0x28166b), ofColor::fromHex(0x192861), ofColor::fromHex(0x040b3c) }); }
  
  
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
  
  void begin(const ofTexture & target)
  {
    isUsingArbTex = ofGetUsingArbTex();
    
    ofEnableArbTex();
    
    getShader().begin();
    getShader().setUniformTexture("palette", getTexture(), 1);
    getShader().setUniform2f("u_paletteSize", getTexture().getWidth(), getTexture().getHeight());
    getShader().setUniform2f("u_resolution", target.getWidth(), target.getHeight());
  }
  
  void end()
  {
    getShader().end();
    
    if (isUsingArbTex) ofEnableArbTex();
    else ofDisableArbTex();
  }
  
  
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
  
  bool isUsingArbTex { false };
  const ofShader & getShader()
  {
    static ofShader shader;
    
    if (!shader.isLoaded())
    {
      std::cout << "Setup Palette-shader" << std::endl;
      
      string vertexShader = "#version 150\n";
      string fragShader = "#version 150\n";
      
      vertexShader += STRINGIFY
      (
       uniform mat4 modelViewProjectionMatrix;
       in vec4 position;
       in vec4 color;
       in vec4 normal;
       in vec2 texcoord;
       
       out vec2 texCoordVarying;
       out vec4 colorVarying;
       
       void main() {
         texCoordVarying = texcoord;
         colorVarying = color;
         gl_Position = modelViewProjectionMatrix * position;
       }
       );
      
      fragShader += STRINGIFY
      (
       float luma(vec3 color) {
         return dot(color, vec3(0.299, 0.587, 0.114));
       }

       float luma(vec4 color) {
         return dot(color.rgb, vec3(0.299, 0.587, 0.114));
       }
       
       in vec4 colorVarying;
       in vec2 texCoordVarying;
       out vec4 outputColor;
       
       uniform sampler2DRect tex0;
       uniform sampler2DRect palette;
       uniform vec2 u_paletteSize;
       
       uniform vec2 u_resolution;
       
       void main() {
         vec2 st = texCoordVarying;
         vec2 uv = st / u_resolution;
         
         vec4 sample = texture(tex0, st);
         float brightness = luma(sample);
         vec4 paletteColor = texture(palette, vec2(brightness, 0) * u_paletteSize);
         
         outputColor = paletteColor;
       }
       );
      
      shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
      shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
      shader.bindDefaults();
      shader.linkProgram();
    }
    
    return shader;
  }
  
  bool isDirty { true };
  
};

}}}
