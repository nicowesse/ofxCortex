#pragma once

#include "ofShader.h"
#include <sstream>

#define STRINGIFY(x) #x

namespace ofxCortex { namespace core { namespace utils {

namespace Shader {

static inline std::string defaultVertexShader = std::string("#version 150\n") + STRINGIFY
 (
  uniform mat4 modelViewProjectionMatrix;
  in vec4 position;
  in vec4 color;
  in vec4 normal;
  in vec2 texcoord;
  
  out vec2 texCoordVarying;
  out vec4 normalVarying;
  out vec4 colorVarying;
  
  void main() {
    texCoordVarying = texcoord;
    colorVarying = color;
    normalVarying = normal;
    gl_Position = modelViewProjectionMatrix * position;
  }
  
  );

static inline std::string defaultFragmentShader = std::string("#version 150\n") + STRINGIFY
 (
  uniform vec4 globalColor;
  
  in vec2 texCoordVarying;
  in vec4 colorVarying;
  
  out vec4 fragColor;
  
  void main() {
    fragColor = globalColor;
  }
  
  );

static inline std::string uvDebug = std::string("#version 150\n") + STRINGIFY
 (
  in vec2 texCoordVarying;
  in vec4 colorVarying;
  
  out vec4 fragColor;
  
  void main() {
    fragColor = vec4(texCoordVarying.x, texCoordVarying.y, 0.0, 1.0);
  }
  
  );

static inline std::string checkers(int divisions = 20) 
{
  std::stringstream shaderStream;
  shaderStream << "#version 150\n" << STRINGIFY
  (
   in vec2 texCoordVarying;
   in vec4 colorVarying;
   
   out vec4 fragColor;
   
   bool inTile(vec2 p, float tileSize) {
     vec2 ptile = step(0.5, fract(0.5 * p / tileSize));
     return ptile.x == ptile.y;
   }
   
   void main() {
     fragColor = vec4(inTile(texCoordVarying, 1.0 / float(DIVISIONS)));
   }
   );
  
  std::string shader = shaderStream.str();
  // Replace "DIVISIONS" with the actual divisions value
  std::string divisionsStr = std::to_string(divisions);
  size_t pos = shader.find("DIVISIONS");
  if (pos != std::string::npos) {
    shader.replace(pos, 9, divisionsStr);  // Replace the placeholder with the actual divisions value
  }
  
  return shader;
}




}}}}
