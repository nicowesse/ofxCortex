#pragma once

#include "ofMain.h"
#include "ofxCortex/utils/ShapingUtils.h"
#include "ofxCortex/utils/GraphicUtils.h"

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
    
    float details { 0.5f };
    float roughness { 1.5f };
    int octaves { 3 };
    
    float contrast { 1.0f };
    float contrastBias { 0.5f };
    
    int seed { 80052 };
    
    void print();
  };
  
  PerlinNoise(const std::string & name = "Perlin Noise") {
    parameters.setName(name);
    parameters.add(seed, uniformScale, scale, details, roughness, octaves, contrast, contrastBias);
  };
  ~PerlinNoise() { _getPerlinShader().unload(); }
  
  operator ofParameterGroup&() { return parameters; }
  const ofParameterGroup& getParameters() const { return parameters; }
  
  glm::vec3 getScale() const { return glm::vec3(scale); }
  float getUniformScale() const { return uniformScale.get(); }
  
protected:
  static ofShader & _getPerlinShader();
  
public:
  double sample(const glm::vec3 & sample) const { return sampleNoise(sample, glm::vec3(scale.get()) * uniformScale.get(), contrast.get(), contrastBias.get(), details.get(), roughness.get(), octaves.get(), seed.get()); }
  double sample(const glm::vec3 & sample, const glm::vec3 & scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80052) const { return sampleNoise(sample, scale, contrast, contrastBias, details, roughness, octaves, seed); }
  
  static double sampleNoise(const glm::vec3 & sample, PerlinNoise::Settings settings);
  static double sampleNoise(const glm::vec3 & sample, const glm::vec3 & scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80052);
  
#pragma mark - Noise - Pixel/Image Methods
  static void begin(const glm::vec2 & resolution, const glm::vec3 & offset, const PerlinNoise::Settings & settings);
  static void end();
  
  static void writeToFbo(ofFbo &fbo, const glm::vec3 & offset, const PerlinNoise::Settings & settings);
  static void writeToFbo(ofFbo &fbo, const glm::vec3 & offset, const glm::vec3 & scale, float contrast = 1.0f, float contrastBias = 0.5f, float details = 0.5f, float roughness = 1.5f, int octaves = 3, int seed = 80057);
  
#pragma mark - Noise - Parameters
protected:
  ofParameterGroup parameters;
  
  ofParameter<int> seed { "Seed", 80052, 0, 100000 };
  ofParameter<float> uniformScale { "Uniform Scale", 1, 0, 1000 };
  ofParameter<glm::vec3> scale { "Scale", glm::vec3(1), glm::vec3(0), glm::vec3(1000) };
  ofParameter<float> details { "Details", 0.5f, 0.0f, 1.0f };
  ofParameter<float> roughness { "Roughness", 1.5f, 1.0f, 2.0f };
  ofParameter<int> octaves { "Octaves", 3, 1, 8 };
  
  ofParameter<float> contrast { "Contrast", 1.0f, 0.0f, 2.0f };
  ofParameter<float> contrastBias { "Contrast Bias", 0.5f, 0.0f, 1.0f };
  
};

}}}
