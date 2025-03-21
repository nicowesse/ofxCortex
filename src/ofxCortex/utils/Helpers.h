#pragma once

#include <random>
#include <chrono>

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofBaseTypes.h"
#include "ofAppGLFWWindow.h"

#include "ofxCortex/utils/ContainerUtils.h"

namespace ofxCortex {

inline static float getScale() {
  static float scale = ((ofAppGLFWWindow*)ofGetWindowPtr())->getPixelScreenCoordScale();
  return scale;
}

}


namespace ofxCortex { namespace core { namespace utils {


inline static ofRectangle getImageRectangle(const ofBaseDraws & image) { return ofRectangle(0, 0, image.getWidth(), image.getHeight()); }

inline static ofRectangle getBitmapStringBoundingBox(const std::string & text)
{
  std::vector<std::string> lines = ofSplitString(text, "\n");
  int maxLineLength = 0;
  for (int i = 0; i < lines.size(); i++) {
    // tabs are not rendered
    const std::string & line(lines[i]);
    int currentLineLength = 0;
    for (int j = 0; j < line.size(); j++) {
      if (line[j] == '\t') {
        currentLineLength += 8 - (currentLineLength % 8);
      } else {
        currentLineLength++;
      }
    }
    maxLineLength = MAX(maxLineLength, currentLineLength);
  }
  
  int padding = 4;
  int fontSize = 8;
  float leading = 1.7;
  int height = lines.size() * fontSize * leading - 1;
  int width = maxLineLength * fontSize;
  return ofRectangle(0, 0, width, height);
}


inline static float getNormalizedTime(float hours = ofGetHours(), float minutes = ofGetMinutes(), float seconds = ofGetSeconds())
{
  float hoursToSeconds = hours * 3600;
  float minutesToSeconds = minutes * 60.0;
  
  return (hoursToSeconds + minutesToSeconds + seconds) / 86400.0;
}

inline static std::tuple<int, int, int> fromNormalizedTime(float t)
{
  t = ofWrap(t, 0, 1);
  
  float totalSeconds = t * 86400.0;
  
  int hours = floor(totalSeconds / 3600.0);
  totalSeconds -= hours * 3600;
  
  int minutes = floor(totalSeconds / 60.0);
  totalSeconds -= minutes * 60.0;
  
  int seconds = totalSeconds;
  
  return { hours, minutes, seconds };
}


template<typename T>
inline static T interpolate(const std::vector<T> & values, float t)
{
  if (values.size() == 1 || ofIsFloatEqual(t, 0.0f)) return values[0];
  if (ofIsFloatEqual(t, 1.0f)) return values[values.size() - 1];
  
  float valueT = t * (values.size() - 1);
  int lower = floor(valueT);
  int upper = ceil(valueT);
  float interpolateT = ofClamp(valueT - lower, 0.0, 1.0);
  
  return ofInterpolateCosine(values[lower], values[upper], interpolateT);
}

template<typename T>
inline static T slerp(const T & A, const T & B, float t)
{
     float dot = glm::dot(A, B);
     glm::clamp(dot, -1.0f, 1.0f);
  
     float theta = acos(dot) * t;
     T relative = glm::normalize(B - A * dot);
  
     return ((A * cos(theta)) + (relative * sin(theta)));
}



#pragma mark - String
namespace String {

inline static std::string toTitleCase(const std::string& str)
{
  std::string result = str;
  bool makeUpper = true; // Flag to indicate the next character should be capitalized
  
  for (char& c : result) {
    if (makeUpper && std::isalpha(c)) {
      c = std::toupper(c); // Capitalize if it's an alphabet character and flag is set
      makeUpper = false;   // Reset the flag
    } else if (std::isspace(c) || c == '-') {
      makeUpper = true;    // Set the flag for the next character
    }
  }
  
  return result;
}

}



#pragma mark - Color
namespace Color {

inline static ofColor hexToColor(const std::string & hex)
{
  unsigned int hexValue;
  std::istringstream iss(hex);
  iss >> std::hex >> hexValue;
  
  int r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
  int g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
  int b = ((hexValue) & 0xFF);
  
  return ofColor(r, g, b);
}

inline static std::string colorToHex(const ofColor& color) {
  std::stringstream hexColor;
  hexColor << "#" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)color.r
           << std::setw(2) << std::setfill('0') << std::uppercase <<  (int)color.g
           << std::setw(2) << std::setfill('0') << std::uppercase <<  (int)color.b;
  
    return hexColor.str();
}

inline static std::vector<ofColor> fromString(const std::string & colors)
{
  return utils::Array::transform<ofColor>(ofSplitString(colors, "-"), &Color::hexToColor);
}

inline static std::vector<ofColor> fromCoolors(const std::string & URL)
{
  return fromString(ofSplitString(URL, "/", true, true).back());
}

inline static ofFloatColor proceduralPalette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
  glm::vec3 color = a + b * cos(TWO_PI * (c * ofWrap(t, 0.0, 1.0) + d));
  return ofFloatColor(color.r, color.g, color.b);
}

inline static ofFloatColor rainbowPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.33, 0.67)); }
inline static ofFloatColor metallicPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.1, 0.2)); }

}



#pragma mark - Performance
namespace Performance {

template<typename Func, typename... Args>
unsigned int measure(Func&& func, Args&&... args)
{
  auto start = std::chrono::high_resolution_clock::now();
  std::forward<Func>(func)(std::forward<Args>(args)...);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  return static_cast<unsigned int>(duration.count());
}

}



#pragma mark - Files
namespace Files {

template<typename F>
inline static void traverse(const std::string & path, const F& func)
{
  ofDirectory dir(path);
  const auto & files = dir.getFiles();
  
  for (const ofFile & file : files)
  {
    if (file.isDirectory()) {
      func(file);
      traverse(file.path(), func);
    }
    else if (file.isFile()) func(file);
  }
}

}

}}}
