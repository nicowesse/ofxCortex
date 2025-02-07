#include "Noise.h"

#define STRINGIFY(x) #x

namespace ofxCortex { namespace core { namespace generators {

void Noise::Settings::print()
{
  cout << "ofxVoid::math::Noise::Settings"
  << "\n Type: " << type
  << "\n Seed: " << seed
  << "\n Offset: " << offset
  << "\n Scale: " << scale
  << "\n Contrast: " << contrast
  << "\n Contrast bias: " << contrastBias
  << "\n Details: " << perlin.details
  << "\n Roughness: " << perlin.roughness
  << "\n Octaves: " << perlin.octaves
  << endl;
}

ofShader & Noise::_getPerlinShader()
{
  static ofShader shader;
  
  if (!shader.isLoaded())
  {
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
     float easing(float x, float amplitude, float shift){
       shift = pow((1.0 - shift) + 0.5, 6.0);
       return pow(x, amplitude) / (pow(x, amplitude) + pow(shift - shift * x, amplitude));
     }
     
     float SimplexPerlin3D( vec3 P )
     {
       //  https://github.com/BrianSharpe/Wombat/blob/master/SimplexPerlin3D.glsl
       
       //  simplex math constants
       const float SKEWFACTOR = 1.0/3.0;
       const float UNSKEWFACTOR = 1.0/6.0;
       const float SIMPLEX_CORNER_POS = 0.5;
       const float SIMPLEX_TETRAHADRON_HEIGHT = 0.70710678118654752440084436210485;    // sqrt( 0.5 )
       
       //  establish our grid cell.
       P *= SIMPLEX_TETRAHADRON_HEIGHT;    // scale space so we can have an approx feature size of 1.0
       vec3 Pi = floor( P + dot( P, vec3( SKEWFACTOR) ) );
       
       //  Find the vectors to the corners of our simplex tetrahedron
       vec3 x0 = P - Pi + dot(Pi, vec3( UNSKEWFACTOR ) );
       vec3 g = step(x0.yzx, x0.xyz);
       vec3 l = 1.0 - g;
       vec3 Pi_1 = min( g.xyz, l.zxy );
       vec3 Pi_2 = max( g.xyz, l.zxy );
       vec3 x1 = x0 - Pi_1 + UNSKEWFACTOR;
       vec3 x2 = x0 - Pi_2 + SKEWFACTOR;
       vec3 x3 = x0 - SIMPLEX_CORNER_POS;
       
       //  pack them into a parallel-friendly arrangement
       vec4 v1234_x = vec4( x0.x, x1.x, x2.x, x3.x );
       vec4 v1234_y = vec4( x0.y, x1.y, x2.y, x3.y );
       vec4 v1234_z = vec4( x0.z, x1.z, x2.z, x3.z );
       
       // clamp the domain of our grid cell
       Pi.xyz = Pi.xyz - floor(Pi.xyz * ( 1.0 / 69.0 )) * 69.0;
       vec3 Pi_inc1 = step( Pi, vec3( 69.0 - 1.5 ) ) * ( Pi + 1.0 );
       
       //  generate the random vectors
       vec4 Pt = vec4( Pi.xy, Pi_inc1.xy ) + vec2( 50.0, 161.0 ).xyxy;
       Pt *= Pt;
       vec4 V1xy_V2xy = mix( Pt.xyxy, Pt.zwzw, vec4( Pi_1.xy, Pi_2.xy ) );
       Pt = vec4( Pt.x, V1xy_V2xy.xz, Pt.z ) * vec4( Pt.y, V1xy_V2xy.yw, Pt.w );
       const vec3 SOMELARGEFLOATS = vec3( 635.298681, 682.357502, 668.926525 );
       const vec3 ZINC = vec3( 48.500388, 65.294118, 63.934599 );
       vec3 lowz_mods = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + Pi.zzz * ZINC.xyz ) );
       vec3 highz_mods = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + Pi_inc1.zzz * ZINC.xyz ) );
       Pi_1 = ( Pi_1.z < 0.5 ) ? lowz_mods : highz_mods;
       Pi_2 = ( Pi_2.z < 0.5 ) ? lowz_mods : highz_mods;
       vec4 hash_0 = fract( Pt * vec4( lowz_mods.x, Pi_1.x, Pi_2.x, highz_mods.x ) ) - 0.49999;
       vec4 hash_1 = fract( Pt * vec4( lowz_mods.y, Pi_1.y, Pi_2.y, highz_mods.y ) ) - 0.49999;
       vec4 hash_2 = fract( Pt * vec4( lowz_mods.z, Pi_1.z, Pi_2.z, highz_mods.z ) ) - 0.49999;
       
       //  evaluate gradients
       vec4 grad_results = inversesqrt( hash_0 * hash_0 + hash_1 * hash_1 + hash_2 * hash_2 ) * ( hash_0 * v1234_x + hash_1 * v1234_y + hash_2 * v1234_z );
       
       //  Normalization factor to scale the final result to a strict 1.0->-1.0 range
       //  http://briansharpe.wordpress.com/2012/01/13/simplex-noise/#comment-36
       const float FINAL_NORMALIZATION = 37.837227241611314102871574478976;
       
       //  evaulate the kernel weights ( use (0.5-x*x)^3 instead of (0.6-x*x)^4 to fix discontinuities )
       vec4 kernel_weights = v1234_x * v1234_x + v1234_y * v1234_y + v1234_z * v1234_z;
       kernel_weights = max(0.5 - kernel_weights, 0.0);
       kernel_weights = kernel_weights*kernel_weights*kernel_weights;
       
       //  sum with the kernel and return
       return dot( kernel_weights, grad_results ) * FINAL_NORMALIZATION * 0.5 + 0.5;
     }
     
     float getNoise(vec3 sample, float seed, float scale, int octaves, float details, float roughness, float contrast, float contrastShift) {
       float value = 0.0;
       const int numOctaves = 10;
       float amplitude = 1.0;
       float frequency = 1.0;
       float noiseHeight = 0.0;
       float maxValue = 0.0;
       
       for (int i = 0; i < numOctaves; i++) {
         if (i > octaves) break;
         
         vec3 sam = sample.xyz / scale * frequency + seed;
         
         float noiseValue = SimplexPerlin3D(sam);
         
         noiseHeight += noiseValue * amplitude;
         maxValue += amplitude;
         
         frequency *= roughness;
         amplitude *= clamp(details, 0.0, 1.0);
       }
       value = clamp(easing(noiseHeight / maxValue, contrast, contrastShift), 0.0, 1.0);
       return value;
     }
     
     in vec4 colorVarying;
     in vec2 texCoordVarying;
     out vec4 outputColor;
     
     uniform sampler2D tex0;
     uniform vec2 u_resolution;
     uniform float u_seed;
     uniform vec3 u_offset;
     uniform float u_scale;
     uniform int   u_octaves;
     uniform float u_details;
     uniform float u_roughness;
     uniform float u_contrast;
     uniform float u_contrastBias;
     
     uniform bool u_useTexture;
     
     void main() {
       vec2 st = (texCoordVarying - 0.5) / u_scale + 0.5;
       
       vec4 sample = vec4(1);
       if (u_useTexture) sample = texture(tex0, texCoordVarying);
       
       sample *= getNoise(vec3(st, 0) + (u_offset / vec3(u_resolution, 1)), u_seed, 1.0, u_octaves, u_details, u_roughness, u_contrast, u_contrastBias);
       sample.a = 1.0;
       
       outputColor = sample;
       //outputColor = vec4(vec3(st.y), 1.0);
       //        outputColor = vec4(st, 0.0, 1.0);
     }
     );
    
    shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
    shader.bindDefaults();
    shader.linkProgram();
  }
  
  return shader;
}

double Noise::getNoise(const glm::vec3 & sample, Noise::Settings settings)
{
  settings.scale = MAX(settings.scale, 0.0001);
  settings.perlin.octaves = MAX(settings.perlin.octaves, 1);
  
  double amplitude = 1.0;
  double frequency = 1.0;
  double noiseHeight = 0.0;
  double maxValue = 0.0;
  
  for (int i = 0; i < settings.perlin.octaves; i++)
  {
    glm::vec3 sample = (sample + (float) settings.seed) / settings.scale * frequency;
    
    double noiseValue = ofNoise(sample);
    noiseHeight += noiseValue * amplitude;
    
    maxValue += amplitude;
    
    frequency *= settings.perlin.roughness; // The frequency of peaks increases each octave. Also known as lacunarity. Values in the 1-X range
    amplitude *= settings.perlin.details; // While the amplitude/heigh/strength decreases each octave. Also known as persistence. Values in the 0-1 range
  }
  
  return utils::Shaping::biasedGain(noiseHeight / maxValue, settings.contrast, settings.contrastBias);
}

double Noise::getNoise(const glm::vec3 &sample, const ofParameterGroup &parameters)
{
  Noise::Settings settings = settingsFromParameters(parameters);
  return getNoise(sample, settings);
}

double Noise::getNoise(glm::vec3 sample, float scale, float contrast, float contrastBias, float details, float roughness, int octaves, int seed)
{
  Noise::Settings settings;
  settings.seed = seed;
  settings.scale = scale;
  settings.contrast = contrast;
  settings.contrastBias = contrastBias;
  settings.perlin.details = details;
  settings.perlin.roughness = roughness;
  settings.perlin.octaves = octaves;
  
  return getNoise(sample, settings);
}

void Noise::begin(glm::vec2 resolution, Noise::Settings settings, bool useTexture)
{
  const ofShader & shader = _getPerlinShader();
  
  shader.begin();
  shader.setUniform2f("u_resolution", resolution);
  shader.setUniform3f("u_offset", settings.offset);
  shader.setUniform1f("u_seed", settings.seed);
  shader.setUniform1f("u_scale", settings.scale / hypot(resolution.x, resolution.y) );
  shader.setUniform1f("u_contrast", settings.contrast);
  shader.setUniform1f("u_contrastBias", settings.contrastBias);
  shader.setUniform1i("u_octaves", settings.perlin.octaves);
  shader.setUniform1f("u_details", settings.perlin.details);
  shader.setUniform1f("u_roughness", settings.perlin.roughness);
  shader.setUniform1i("u_useTexture", useTexture);
}

void Noise::end(Noise::Settings settings)
{
  _getPerlinShader().end();
}

void Noise::writeNoiseToFbo(ofFbo &fbo, Noise::Settings settings)
{
  if (!fbo.isAllocated())
  {
    ofLogWarning("ofxCortex::generators::Noise") << "The FBO is not allocated. Please allocate before using";
    return;
  }
  
  int w = fbo.getWidth();
  int h = fbo.getHeight();
  
  fbo.begin();
  {
    ofClear(0, 0, 0, 0);
    
    ofSetColor(255);
    Noise::begin(glm::vec2(w, h), settings, false);
    ofxCortex::core::utils::Graphics::drawTexCoordRectangle(0, 0, w, h);
    Noise::end(settings);
  }
  fbo.end();
}

void Noise::writeNoiseToFbo(ofFbo &fbo, const glm::vec3 & offset, float scale, float contrast, float contrastBias, float details, float roughness, int octaves, int seed)
{
  Noise::Settings settings;
  settings.seed = seed;
  settings.offset = offset;
  settings.scale = scale;
  settings.contrast = contrast;
  settings.contrastBias = contrastBias;
  settings.perlin.details = details;
  settings.perlin.roughness = roughness;
  settings.perlin.octaves = 1;
  
  writeNoiseToFbo(fbo, settings);
}

void Noise::settingsFromParameters(Noise::Settings &settings, const ofParameterGroup &parameters)
{
  ofParameterGroup noiseParameters = parameters.getGroup("Noise Settings");
  settings.seed = noiseParameters.get<int>("Seed").get();
  
  settings.scale = noiseParameters.get<float>("Scale").get();
  settings.contrast = noiseParameters.get<float>("Contrast").get();
  settings.contrastBias = noiseParameters.get<float>("Contrast Bias").get();
  
  settings.perlin.details = noiseParameters.getGroup("Perlin").get<float>("Details").get();
  settings.perlin.roughness = noiseParameters.getGroup("Perlin").get<float>("Roughness").get();
  settings.perlin.octaves = noiseParameters.getGroup("Perlin").get<int>("Octaves").get();
  
  settings.voronoi.type = "F1"; //noiseParameters.getGroup("Voronoi").get<string>("Type").get();
  settings.voronoi.squareness = noiseParameters.getGroup("Voronoi").get<float>("Squareness").get();
}

Noise::Settings Noise::settingsFromParameters(const ofParameterGroup &parameters)
{
  Noise::Settings settings;
  settingsFromParameters(settings, parameters);
  return settings;
}

void Noise::addParameters(ofParameterGroup &parameters)
{
  vector<string> types{ "Perlin", "Cellular", "Voronoi" };
  //  ofParameter<ofxVoid::types::Select<string>> type("Type", ofxVoid::types::Select<string>(types[0], types));
  
  
  //  ofParameter<glm::vec3> speed("Speed", glm::vec3(0, 0, 1), glm::vec3(-30, -30, -30), glm::vec3(30, 30, 30));
  ofParameter<float> speedX("Speed X", 0, -30, 30);
  ofParameter<float> speedY("Speed Y", 0, -30, 30);
  ofParameter<float> speedZ("Speed Z", 0, -30, 30);
  ofParameter<int> seed("Seed", 80085, 1, 100000);
  ofParameter<float> scale("Scale", 40.0f, 1.0f, 5000.0f);
  ofParameter<float> contrast("Contrast", 0.5f, 0.0f, 10.0f);
  ofParameter<float> contrastShift("Contrast Bias", 0.5f, 0.0f, 1.0f);
  
  
  ofParameter<float> details("Details", 0.5f, 0.0f, 1.0f);
  ofParameter<float> roughness("Roughness", 1.5f, 1.0f, 2.0f);
  ofParameter<int> octaves("Octaves", 3, 1, 10);
  ofParameterGroup perlinSettings {
    "Perlin",
    details,
    roughness,
    octaves
  };
  
  ofParameterGroup cellularSettings{
    "Cellular"
  };
  
  vector<string> voronoiTypes{ "F1", "F2", "F2-F1", "F1*F2" };
  //  ofParameter<ofxVoid::types::Select<string>> voronoiType("Type", ofxVoid::types::Select<string>(voronoiTypes[0], voronoiTypes));
  ofParameter<float> squareness("Squareness", 0.0, 0.0, 1.0);
  ofParameterGroup voronoiSettings {
    "Voronoi",
    //    voronoiType,
    squareness
  };
  
  ofParameterGroup noiseGroup{
    "Noise Settings",
    //    type,
    speedX, speedY, speedZ,
    seed,
    scale,
    contrast,
    contrastShift,
    perlinSettings,
    cellularSettings,
    voronoiSettings
  };
  
  parameters.add(noiseGroup);
}

ofParameterGroup Noise::getParameters()
{
  ofParameterGroup parameters;
  addParameters(parameters);
  return parameters;
}

#pragma mark - PerlinNoise


double PerlinNoise::sampleNoise(const glm::vec3 & sample, PerlinNoise::Settings settings)
{
  settings.scale = glm::max(settings.scale, glm::vec3(0.0001));
  settings.octaves = MAX(settings.octaves, 1);
  
  double amplitude = 1.0;
  double frequency = 1.0;
  double noiseHeight = 0.0;
  double maxValue = 0.0;
  
  for (int i = 0; i < settings.octaves; i++)
  {
    glm::vec3 lookup = (sample + (float) settings.seed) / settings.scale * frequency;
    
    double noiseValue = ofNoise(lookup);
    noiseHeight += noiseValue * amplitude;
    
    maxValue += amplitude;
    
    frequency *= settings.roughness; // The frequency of peaks increases each octave. Also known as lacunarity. Values in the 1-X range
    amplitude *= settings.details; // While the amplitude/heigh/strength decreases each octave. Also known as persistence. Values in the 0-1 range
  }
  
  return utils::Shaping::biasedGain(noiseHeight / maxValue, settings.contrast, settings.contrastBias);
}

double PerlinNoise::sampleNoise(const glm::vec3 & sample, const glm::vec3 & scale, float contrast, float contrastBias, float details, float roughness, int octaves, int seed)
{
  PerlinNoise::Settings settings;
  settings.scale = glm::vec3(scale);
  settings.seed = seed;
  settings.contrast = contrast;
  settings.contrastBias = contrastBias;
  settings.details = details;
  settings.roughness = roughness;
  settings.octaves = octaves;
  
  return sampleNoise(sample, settings);
}

void PerlinNoise::begin(const glm::vec2 & resolution, const glm::vec3 & offset, const PerlinNoise::Settings & settings)
{
  const ofShader & shader = _getPerlinShader();
  
  shader.begin();
  shader.setUniform2f("u_resolution", resolution);
  shader.setUniform3f("u_offset", offset);
  shader.setUniform1f("u_seed", settings.seed);
  shader.setUniform3f("u_scale", settings.scale);
  shader.setUniform1f("u_contrast", settings.contrast);
  shader.setUniform1f("u_contrastBias", settings.contrastBias);
  shader.setUniform1i("u_octaves", settings.octaves);
  shader.setUniform1f("u_details", settings.details);
  shader.setUniform1f("u_roughness", settings.roughness);
}

void PerlinNoise::end()
{
  _getPerlinShader().end();
}

void PerlinNoise::writeToFbo(ofFbo &fbo, const glm::vec3 & offset, const PerlinNoise::Settings & settings)
{
  if (!fbo.isAllocated())
  {
    ofLogWarning("ofxCortex::generators::PerlinNoise") << "The FBO is not allocated. Please allocate before using";
    return;
  }
  
  if (!_getPerlinShader().isLoaded())
  {
    ofLogWarning("ofxCortex::generators::PerlinNoise") << "The shader is not loaded.";
    return;
  }
  
  int w = fbo.getWidth();
  int h = fbo.getHeight();
  
  fbo.begin();
  {
    ofClear(0, 0, 0, 0);
    
    PerlinNoise::begin(glm::vec2(w, h), offset, settings);
    ofxCortex::core::utils::Graphics::drawTexCoordRectangle(0, 0, w, h);
    PerlinNoise::end();
  }
  fbo.end();
}

void PerlinNoise::writeToFbo(ofFbo &fbo, const glm::vec3 & offset, const glm::vec3 & scale, float contrast, float contrastBias, float details, float roughness, int octaves, int seed)
{
  PerlinNoise::Settings settings;
  settings.seed = seed;
  settings.scale = scale;
  settings.contrast = contrast;
  settings.contrastBias = contrastBias;
  settings.details = details;
  settings.roughness = roughness;
  settings.octaves = 1;
  
  PerlinNoise::writeToFbo(fbo, offset, settings);
}

static const string PARAMETER_GROUP_NAME = "Perlin Noise Settings";
static const string SEED_NAME = "Seed";
static const string SCALE_NAME = "Scale";
static const string CONTRAST_NAME = "Contrast";
static const string CONTRAST_BIAS_NAME = "Contrast Bias";
static const string DETAILS_NAME = "Details";
static const string ROUGHNESS_NAME = "Roughness";
static const string OCTAVES_NAME = "Octaves";

//void PerlinNoise::settingsFromParameters(PerlinNoise::Settings &settings, const ofParameterGroup &parameters)
//{
//  if (!parameters.contains(PARAMETER_GROUP_NAME)) return;
//  
//  ofParameterGroup noiseParameters = parameters.getGroup(PARAMETER_GROUP_NAME);
//  
//  settings.seed = noiseParameters.get<int>(SEED_NAME).get();
//  settings.scale = glm::vec3(noiseParameters.get<float>(SCALE_NAME).get());
//  
//  settings.contrast = noiseParameters.get<float>(CONTRAST_NAME).get();
//  settings.contrastBias = noiseParameters.get<float>(CONTRAST_BIAS_NAME).get();
//  
//  settings.details = noiseParameters.get<float>(DETAILS_NAME).get();
//  settings.roughness = noiseParameters.get<float>(ROUGHNESS_NAME).get();
//  settings.octaves = noiseParameters.get<int>(OCTAVES_NAME).get();
//}
//
//PerlinNoise::Settings PerlinNoise::settingsFromParameters(const ofParameterGroup &parameters)
//{
//  PerlinNoise::Settings settings;
//  settingsFromParameters(settings, parameters);
//  return settings;
//}
//
//ofParameterGroup PerlinNoise::addParameters(ofParameterGroup &parameters)
//{
//  ofParameter<int> seed(SEED_NAME, 80085, 1, 100000);
//  ofParameter<float> scale(SCALE_NAME, 40.0f, 1.0f, 5000.0f);
//  ofParameter<float> contrast(CONTRAST_NAME, 0.5f, 0.0f, 10.0f);
//  ofParameter<float> contrastBias(CONTRAST_BIAS_NAME, 0.5f, 0.0f, 1.0f);
//  
//  
//  ofParameter<float> details(DETAILS_NAME, 0.5f, 0.0f, 1.0f);
//  ofParameter<float> roughness(ROUGHNESS_NAME, 1.5f, 1.0f, 2.0f);
//  ofParameter<int> octaves(OCTAVES_NAME, 3, 1, 10);
//  
//  ofParameterGroup noiseGroup{
//    PARAMETER_GROUP_NAME,
//    seed,
//    scale,
//    contrast,
//    contrastBias,
//    octaves,
//    details,
//    roughness,
//  };
//  
//  parameters.add(noiseGroup);
//  
//  return noiseGroup;
//}
//
//ofParameterGroup PerlinNoise::getParameters()
//{
//  ofParameterGroup parameters;
//  addParameters(parameters);
//  return parameters;
//}

ofShader & PerlinNoise::_getPerlinShader()
{
  static ofShader shader;
  
  if (!shader.isLoaded())
  {
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
     float easing(float x, float amplitude, float shift){
       shift = pow((1.0 - shift) + 0.5, 6.0);
       return pow(x, amplitude) / (pow(x, amplitude) + pow(shift - shift * x, amplitude));
     }
     
     float SimplexPerlin3D( vec3 P )
     {
       //  https://github.com/BrianSharpe/Wombat/blob/master/SimplexPerlin3D.glsl
       
       //  simplex math constants
       const float SKEWFACTOR = 1.0/3.0;
       const float UNSKEWFACTOR = 1.0/6.0;
       const float SIMPLEX_CORNER_POS = 0.5;
       const float SIMPLEX_TETRAHADRON_HEIGHT = 0.70710678118654752440084436210485;    // sqrt( 0.5 )
       
       //  establish our grid cell.
       P *= SIMPLEX_TETRAHADRON_HEIGHT;    // scale space so we can have an approx feature size of 1.0
       vec3 Pi = floor( P + dot( P, vec3( SKEWFACTOR) ) );
       
       //  Find the vectors to the corners of our simplex tetrahedron
       vec3 x0 = P - Pi + dot(Pi, vec3( UNSKEWFACTOR ) );
       vec3 g = step(x0.yzx, x0.xyz);
       vec3 l = 1.0 - g;
       vec3 Pi_1 = min( g.xyz, l.zxy );
       vec3 Pi_2 = max( g.xyz, l.zxy );
       vec3 x1 = x0 - Pi_1 + UNSKEWFACTOR;
       vec3 x2 = x0 - Pi_2 + SKEWFACTOR;
       vec3 x3 = x0 - SIMPLEX_CORNER_POS;
       
       //  pack them into a parallel-friendly arrangement
       vec4 v1234_x = vec4( x0.x, x1.x, x2.x, x3.x );
       vec4 v1234_y = vec4( x0.y, x1.y, x2.y, x3.y );
       vec4 v1234_z = vec4( x0.z, x1.z, x2.z, x3.z );
       
       // clamp the domain of our grid cell
       Pi.xyz = Pi.xyz - floor(Pi.xyz * ( 1.0 / 69.0 )) * 69.0;
       vec3 Pi_inc1 = step( Pi, vec3( 69.0 - 1.5 ) ) * ( Pi + 1.0 );
       
       //  generate the random vectors
       vec4 Pt = vec4( Pi.xy, Pi_inc1.xy ) + vec2( 50.0, 161.0 ).xyxy;
       Pt *= Pt;
       vec4 V1xy_V2xy = mix( Pt.xyxy, Pt.zwzw, vec4( Pi_1.xy, Pi_2.xy ) );
       Pt = vec4( Pt.x, V1xy_V2xy.xz, Pt.z ) * vec4( Pt.y, V1xy_V2xy.yw, Pt.w );
       const vec3 SOMELARGEFLOATS = vec3( 635.298681, 682.357502, 668.926525 );
       const vec3 ZINC = vec3( 48.500388, 65.294118, 63.934599 );
       vec3 lowz_mods = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + Pi.zzz * ZINC.xyz ) );
       vec3 highz_mods = vec3( 1.0 / ( SOMELARGEFLOATS.xyz + Pi_inc1.zzz * ZINC.xyz ) );
       Pi_1 = ( Pi_1.z < 0.5 ) ? lowz_mods : highz_mods;
       Pi_2 = ( Pi_2.z < 0.5 ) ? lowz_mods : highz_mods;
       vec4 hash_0 = fract( Pt * vec4( lowz_mods.x, Pi_1.x, Pi_2.x, highz_mods.x ) ) - 0.49999;
       vec4 hash_1 = fract( Pt * vec4( lowz_mods.y, Pi_1.y, Pi_2.y, highz_mods.y ) ) - 0.49999;
       vec4 hash_2 = fract( Pt * vec4( lowz_mods.z, Pi_1.z, Pi_2.z, highz_mods.z ) ) - 0.49999;
       
       //  evaluate gradients
       vec4 grad_results = inversesqrt( hash_0 * hash_0 + hash_1 * hash_1 + hash_2 * hash_2 ) * ( hash_0 * v1234_x + hash_1 * v1234_y + hash_2 * v1234_z );
       
       //  Normalization factor to scale the final result to a strict 1.0->-1.0 range
       //  http://briansharpe.wordpress.com/2012/01/13/simplex-noise/#comment-36
       const float FINAL_NORMALIZATION = 37.837227241611314102871574478976;
       
       //  evaulate the kernel weights ( use (0.5-x*x)^3 instead of (0.6-x*x)^4 to fix discontinuities )
       vec4 kernel_weights = v1234_x * v1234_x + v1234_y * v1234_y + v1234_z * v1234_z;
       kernel_weights = max(0.5 - kernel_weights, 0.0);
       kernel_weights = kernel_weights*kernel_weights*kernel_weights;
       
       //  sum with the kernel and return
       return dot( kernel_weights, grad_results ) * FINAL_NORMALIZATION * 0.5 + 0.5;
     }
     
     float getNoise(vec3 sample, vec3 offset, float seed, vec3 scale, int octaves, float details, float roughness, float contrast, float contrastShift) {
       float value = 0.0;
       const int numOctaves = 10;
       float amplitude = 1.0;
       float frequency = 1.0;
       float noiseHeight = 0.0;
       float maxValue = 0.0;
       
       for (int i = 0; i < numOctaves; i++) {
         if (i > octaves) break;
         
         vec3 sam = (sample + offset + vec3(seed)) / scale * frequency;
         
         float noiseValue = SimplexPerlin3D(sam);
         
         noiseHeight += noiseValue * amplitude;
         maxValue += amplitude;
         
         frequency *= roughness;
         amplitude *= clamp(details, 0.0, 1.0);
       }
       value = clamp(easing(noiseHeight / maxValue, contrast, contrastShift), 0.0, 1.0);
       return value;
     }
     
     in vec4 colorVarying;
     in vec2 texCoordVarying;
     out vec4 outputColor;
     
     uniform vec2 u_resolution;
     uniform float u_seed;
     uniform vec3 u_offset;
     uniform vec3 u_scale;
     uniform int   u_octaves;
     uniform float u_details;
     uniform float u_roughness;
     uniform float u_contrast;
     uniform float u_contrastBias;
     
     uniform bool u_useTexture;
     
     void main() {
       vec2 st = texCoordVarying; //(texCoordVarying - 0.5) / u_scale + 0.5;
       vec2 fragCoord = texCoordVarying * u_resolution;
       
       vec4 noise = vec4(1);
       vec3 sample = vec3(fragCoord, 0.0);
       
       noise.rgb *= getNoise(sample, u_offset, u_seed, u_scale, u_octaves, u_details, u_roughness, u_contrast, u_contrastBias);
       noise.a = 1.0;
       
       outputColor = noise;
//       outputColor = vec4(texCoordVarying.xy, 0.0, 1.0);
       //        outputColor = vec4(st, 0.0, 1.0);
     }
     );
    
    shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragShader);
    shader.bindDefaults();
    shader.linkProgram();
  }
  
  return shader;
}

}}}
