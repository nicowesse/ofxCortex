#pragma once

#include <random>
#include <chrono>

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofBaseTypes.h"

#include "ofxCortex/utils/ContainerUtils.h"


namespace ofxCortex { namespace core { namespace utils {


static ofRectangle getImageRectangle(const ofBaseDraws & image) { return ofRectangle(0, 0, image.getWidth(), image.getHeight()); }

static ofRectangle getBitmapStringBoundingBox(const std::string & text)
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



#pragma mark - String
namespace String {

static std::string toTitleCase(const std::string& str)
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

static ofColor hexToColor(const std::string & hex)
{
  unsigned int hexValue;
  std::istringstream iss(hex);
  iss >> std::hex >> hexValue;
  
  int r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
  int g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
  int b = ((hexValue) & 0xFF);
  
  return ofColor(r, g, b);
}

static std::string colorToHex(const ofColor& color) {
    // Get the individual components of the color
    int r = color.r;
    int g = color.g;
    int b = color.b;

    // Convert each component to its hexadecimal representation
    std::stringstream ss;
    ss << "#" << std::hex << std::setw(2) << std::setfill('0') << r
       << std::hex << std::setw(2) << std::setfill('0') << g
       << std::hex << std::setw(2) << std::setfill('0') << b;
  
  std::cout << "colorToHex = " << ss.str() << std::endl;

    // Return the concatenated hexadecimal string
    return ss.str();
}

static std::vector<ofColor> fromString(const std::string & colors)
{
  return utils::Array::transform<ofColor>(ofSplitString(colors, "-"), &Color::hexToColor);
}

static std::vector<ofColor> fromCoolors(const std::string & URL)
{
  return fromString(ofSplitString(URL, "/", true, true).back());
}

static ofFloatColor proceduralPalette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
  glm::vec3 color = a + b * cos(TWO_PI * (c * ofWrap(t, 0.0, 1.0) + d));
  return ofFloatColor(color.r, color.g, color.b);
}

static ofFloatColor rainbowPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.33, 0.67)); }
static ofFloatColor metallicPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.1, 0.2)); }

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
void traverse(const std::string & path, const F& func)
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
