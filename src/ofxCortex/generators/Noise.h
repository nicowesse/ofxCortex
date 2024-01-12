#pragma once

#include "ofMain.h"
#include "ofxCortex/utils/Shaping.h"
#include "ofxCortex/utils/Helpers.h"

namespace ofxCortex { namespace core { namespace generators {

class Noise {
public:
  struct Settings {
    string type{"Perlin"};
    glm::vec3 offset { 0.0f };
    int seed{80052};
    float scale{1.0f};
    
    float contrast{1.0f};
    float contrastBias{0.5f};
    
    struct {
      float details{0.5f};
      float roughness{1.5f};
      int octaves{3};
    } perlin;
    
    struct {
    } cellular;
    
    struct {
      string type{"F1"};
      float squareness{0.0f};
    } voronoi;
    
    void print();
  };
  
protected:
  Noise() {};
  ~Noise() { _getPerlinShader().unload(); }
  static ofShader & _getPerlinShader();
  
public:
  static double getNoise(const glm::vec3 & sample, Noise::Settings settings);
  
  static double getNoise(const glm::vec3 &sample, const ofParameterGroup &parameters);
  
  static double getNoise(glm::vec3 sample, float scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80052);
  
#pragma mark - Noise - Pixel/Image Methods
  static void begin(glm::vec2 resolution, Noise::Settings settings, bool useTexture = false);
  static void end(Noise::Settings settings);
  
  static void writeNoiseToFbo(ofFbo &fbo, Noise::Settings settings);
  static void writeNoiseToFbo(ofFbo &fbo, const glm::vec3 & offset, float scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80057);
  
#pragma mark - Noise - Settings Helper Methods
  
  static void settingsFromParameters(Noise::Settings &settings, const ofParameterGroup &parameters);
  static Noise::Settings settingsFromParameters(const ofParameterGroup &parameters);
  
  static void addParameters(ofParameterGroup &parameters);
  static ofParameterGroup getParameters();
};




#pragma mark - Perlin Noise
class PerlinNoise {
public:
  struct Settings {
    glm::vec3 scale { 1.0f };
    glm::vec3 offset { 0.0f };
    
    float details { 0.5f };
    float roughness { 1.5f };
    int octaves { 3 };
    
    float contrast { 1.0f };
    float contrastBias { 0.5f };
    
    int seed { 80052 };
    
    void print();
  };
  
protected:
  PerlinNoise() {};
  ~PerlinNoise() { _getPerlinShader().unload(); }
  static ofShader & _getPerlinShader();
  
public:
  static double getNoise(const glm::vec3 & sample, PerlinNoise::Settings settings);
  static double getNoise(const glm::vec3 & sample, const ofParameterGroup &parameters);
  static double getNoise(const glm::vec3 & sample, float scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80052);
  
#pragma mark - Noise - Pixel/Image Methods
  static void begin(glm::vec2 resolution, PerlinNoise::Settings settings);
  static void end();
  
  static void writeToFbo(ofFbo &fbo, PerlinNoise::Settings settings);
  static void writeToFbo(ofFbo &fbo, glm::vec3 offset, glm::vec3 scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80057);
  
#pragma mark - Noise - Settings Helper Methods
  
  static void settingsFromParameters(PerlinNoise::Settings &settings, const ofParameterGroup &parameters);
  static PerlinNoise::Settings settingsFromParameters(const ofParameterGroup &parameters);
  
  static ofParameterGroup addParameters(ofParameterGroup &parameters);
  static ofParameterGroup getParameters();
};

}}}
