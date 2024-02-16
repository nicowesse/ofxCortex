#pragma once

#include "ofConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include "of3dGraphics.h"
#include "of3dUtils.h"
#include "ofGraphics.h"
#include "ofLog.h"
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>
#include "ofxCortex/types/Box.h"
#include "ofxCortex/utils/Numbers.h"
#include "ofxCortex/utils/Shaping.h"
#include <type_traits>


#define ALIAS_TEMPLATE_FUNCTION(highLevelF, lowLevelF) \
template<typename... Args> \
inline auto highLevelF(Args&&... args) -> decltype(lowLevelF(std::forward<Args>(args)...)) \
{ \
    return lowLevelF(std::forward<Args>(args)...); \
}

// Helper to determine whether there's a const_iterator for T.
template<typename T>
struct has_const_iterator
{
private:
  template<typename C> static char test(typename C::const_iterator*);
  template<typename C> static int  test(...);
public:
  enum { value = sizeof(test<T>(0)) == sizeof(char) };
};


namespace ofxCortex { namespace core { namespace utils {

static glm::vec2 randomInRectangle(const ofRectangle & rect)
{
  return { (int) ofRandom(rect.getLeft(), rect.getRight()), (int) ofRandom(rect.getTop(), rect.getBottom()) };
}

static bool isInsideBox(const ofBoxPrimitive & box, const glm::vec3 & p)
{
  return (p.x >= 0.0f && p.x <= box.getWidth()) && (p.y >= 0.0f && p.y <= box.getHeight()) && (p.z >= 0.0f && p.z <= box.getDepth());
}

static bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R)
{
  static const float EPSILON = 0.00000001;
  
  const glm::vec3 & V1 = face.getVertex(0);
  const glm::vec3 & V2 = face.getVertex(1);
  const glm::vec3 & V3 = face.getVertex(2);
  
  glm::vec3 e1, e2; //Edge1, Edge2
  glm::vec3 P, Q, T;
  float det, inv_det, u, v;
  float t;
  
  //Find vectors for two edges sharing V1
  e1 = V2 - V1;
  e2 = V3 - V1;
  //Begin calculating determinant - also used to calculate u parameter
  P = glm::cross(D, e2); //D.getCrossed(e2);
                         //if determinant is near zero, ray lies in plane of triangle
  det = glm::dot(e1, P); //e1.dot(P);
                         //NOT CULLING
  if(det > -EPSILON && det < EPSILON){
    return false;
  }
  inv_det = 1.f / det;
  
  //calculate distance from V1 to ray origin
  T = O - V1;
  
  //Calculate u parameter and test bound
  u = glm::dot(T, P) * inv_det; //T.dot(P) * inv_det;
                                //The intersection lies outside of the triangle
  if(u < 0.f || u > 1.f){
    return false;
  }
  
  //Prepare to test v parameter
  Q = glm::cross(T, e1); //T.getCrossed(e1);
  
  //Calculate V parameter and test bound
  v = glm::dot(D, Q) * inv_det; //D.dot(Q) * inv_det;
                                //The intersection lies outside of the triangle
  if(v < 0.f || u + v  > 1.f){
    return false;
  }
  
  t = glm::dot(e2, Q) * inv_det; //e2.dot(Q) * inv_det;
  
  if(t > EPSILON){ //ray intersection
    R = O + t * D; // store intersection point
    return true;
  }
  
  // No hit, no win
  return false;
}

static bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh)
{
  static bool warning_given;
  if (mesh.getMode() != OF_PRIMITIVE_TRIANGLES)
  {
    if (!warning_given)
    {
      ofLogWarning("isInsideMesh") << "ofMesh mode should be OF_PRIMITIVE_TRIANGLES";
      warning_given = true;
    }
    return false;
  }
  
  glm::vec3 foundIntersection;
  std::vector<glm::vec3> intersections;
  glm::vec3 direction = glm::sphericalRand(1);
  
  for (auto & face : mesh.getUniqueFaces())
  {
    if (rayTriangleIntersection(face, p, direction, foundIntersection)) intersections.push_back(foundIntersection);
  }
  
  std::vector<glm::vec3> unique_intersections;
  std::unique_copy(intersections.begin(), intersections.end(), std::back_inserter(unique_intersections));
  
  return unique_intersections.size() % 2 == 1;
}

static std::vector<glm::vec3> pointsInsideMesh(const std::vector<glm::vec3> & points, const ofMesh & mesh)
{
  std::vector<glm::vec3> output;
  std::copy_if(points.begin(), points.end(), back_inserter(output), [&mesh](const glm::vec3 & p) { return isInsideMesh(p, mesh); });
  return output;
}

static ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh)
{
  glm::vec3 min, max;
  const std::vector<glm::vec3> vertices = mesh.getVertices();
  
  min.x = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.x < b.x; })->x;
  min.y = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.y < b.y; })->y;
  min.z = std::min_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.z < b.z; })->z;
  
  max.x = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.x < b.x; })->x;
  max.y = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.y < b.y; })->y;
  max.z = std::max_element(vertices.begin(), vertices.end(), [](const glm::vec3 & a, const glm::vec3 & b) { return a.z < b.z; })->z;
  
  glm::vec3 size = max - min;
  
  return ofxCortex::core::types::Box(min, size.x, size.y, size.z);
}

static ofRectangle getImageRectangle(const ofBaseDraws & image)
{
  return ofRectangle(0, 0, image.getWidth(), image.getHeight());
}

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

static void drawTexCoordRectangle(float x = 0.0f, float y = 0.0f, float w = 1.0f, float h = 1.0f)
{
  static ofMesh mesh;
  
  if (!mesh.hasVertices())
  {
    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    mesh.addVertex(glm::vec3(0, 0, 0));
    mesh.addTexCoord(glm::vec2(0, 0));
    
    mesh.addVertex(glm::vec3(1, 0, 0));
    mesh.addTexCoord(glm::vec2(1, 0));
    
    mesh.addVertex(glm::vec3(0, 1, 0));
    mesh.addTexCoord(glm::vec2(0, 1));
    
    mesh.addVertex(glm::vec3(1, 1, 0));
    mesh.addTexCoord(glm::vec2(1, 1));
    
    //    mesh.addTriangle(0, 1, 2);
    //    mesh.addTriangle(2, 3, 0);
  }
  
  ofPushMatrix();
  ofTranslate(x, y);
  ofScale(w, h);
  mesh.draw();
  ofPopMatrix();
}

static void drawTexCoordRectangle(const ofRectangle & rect) 
{
  drawTexCoordRectangle(rect.x, rect.y, rect.width, rect.height);
}

static float getNormalizedTime(float hours = ofGetHours(), float minutes = ofGetMinutes(), float seconds = ofGetSeconds())
{
  float hoursToSeconds = hours * 3600;
  float minutesToSeconds = minutes * 60.0;
  
  return (hoursToSeconds + minutesToSeconds + seconds) / 86400.0;
}



#pragma mark - Map
namespace Map {

template<typename Map>
std::vector<typename Map::key_type> keys(const Map& m) 
{
  std::vector<typename Map::key_type> keys;
  for (const auto& pair : m) {
    keys.push_back(pair.first);
  }
  return keys;
}

template<typename Map>
std::vector<typename Map::value_type> values(const Map& m) 
{
  std::vector<typename Map::value_type> keys;
  for (const auto& pair : m) {
    keys.push_back(pair.second);
  }
  return keys;
}

}



#pragma mark - Array
namespace Array {

template<typename T>
static T & atWrapped(std::vector<T> & v, int index)
{
  return v[modulo(index, v.size())];
}

template<typename T>
static T randomInVector(const std::vector<T> & v)
{
  auto it = v.cbegin();
  int random = rand() % v.size();
  std::advance(it, random);
  
  return *it;
}

ALIAS_TEMPLATE_FUNCTION(sample, randomInVector)

template<typename T>
std::vector<T> exclude(const std::vector<T>& input, const std::vector<T>& exclude) 
{
  std::vector<T> result;
  std::copy_if(input.cbegin(), input.cend(), std::back_inserter(result), [&](const T& element) {
    return std::find(exclude.cbegin(), exclude.cend(), element) == exclude.cend();
  });
  return result;
}

template<typename T>
static void remove(std::vector<T>& source, const std::vector<T>& remove) 
{
  source.erase(std::remove_if(source.begin(), source.end(), [&remove](const T& item) {
    return std::find(remove.cbegin(), remove.cend(), item) != remove.cend();
  }), source.end());
}

template<typename T>
static T randomInVectorExcept(const std::vector<T> &v, const std::vector<T>& except) { return randomInVector(exclude(v, except)); }

ALIAS_TEMPLATE_FUNCTION(sampleExcept, randomInVectorExcept)

template<typename T>
static T sampleExcept(const std::vector<T> & v, const T& except)
{
  return randomInVectorExcept(v, { except });
}

template <template <typename, typename> class Container, typename T, typename Allocator = std::allocator<T>>
void subtractFromVector(Container<T, Allocator>& input, const Container<T, Allocator>& subtract) 
{
  input.erase(std::remove_if(input.begin(), input.end(), [&subtract](T element) { return std::find(subtract.begin(), subtract.end(), element) != subtract.end(); }), input.end());
}

template<typename T>
static std::vector<T> constructVector(int n, std::function<T(int)> func = [](int index) { return T(); })
{
  std::vector<T> output(n);
  for (int i = 0; i < n; i++) output[i] = func(i);
//  std::generate(output.begin(), output.end(), func);
  return output;
}

template<typename T>
static std::vector<T> constructVector(int columns, int rows, std::function<T(int, int)> func = [](int col, int row) { return T(); })
{
  std::vector<T> output(columns * rows);
  for (int row = 0; row < rows; row++)
  {
    for (int column = 0; column < columns; column++) { output[column + row * columns] = func(column, row); }
  }
  return output;
}

template<typename T>
static std::vector<T> flatten(const std::vector<std::vector<T>> & v)
{
  std::vector<T> output;
  for(const auto & x : v)
    output.insert(output.end(), x.begin(), x.end());
  return output;
}

template<typename T>
static int findIndex(const std::vector<T> & v, const T & needle)
{
  auto it = find(begin(v), end(v), needle);
  return (it != v.end()) ? it - v.begin() : -1;
}

template<typename T>
static std::vector<unsigned int> findIndices(const std::vector<T> & v, const std::vector<T> & needles)
{
  std::vector<unsigned int> indices;
  for (const T & needle : needles)
  {
    int index = findIndex(v, needle);
    if (index != -1) indices.push_back(index);
  }
  return indices;
}

template <typename T>
static std::vector<T> randomSubset(const std::vector<T>& v, std::size_t size) 
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<std::size_t> indices(v.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::shuffle(indices.begin(), indices.end(), gen);
  std::partial_sort(indices.begin(), indices.begin() + size, indices.end(),
                    [&gen](std::size_t i, std::size_t j) { return gen() < gen(); });
  std::vector<T> result(size);
  std::transform(indices.begin(), indices.begin() + size, result.begin(),
                 [&v](std::size_t i) { return v[i]; });
  return result;
}

ALIAS_TEMPLATE_FUNCTION(sampleN, randomSubset)

//template<typename T>
template<typename T, template<typename, typename> class Container, typename Allocator = std::allocator<T>>
static void appendVector(Container<T, Allocator> & original, Container<T, Allocator> & appending)
{
  original.insert(std::end(original), std::begin(appending), std::end(appending));
}

ALIAS_TEMPLATE_FUNCTION(append, appendVector)

template<typename OutputType, typename InputType, typename Func>
static std::vector<OutputType> transform(const std::vector<InputType> & v, Func func)
{
  std::vector<OutputType> output;
  std::transform(begin(v), end(v), std::back_inserter(output), func);
  return output;
}

ALIAS_TEMPLATE_FUNCTION(map, transform)

//template<typename T, template <typename, typename...> class Container, typename... Args, typename Func, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
//static std::vector<T> filter(const Container<T, Args...> & v, Func func)
//{
//  std::vector<T> output;
//  std::copy_if(begin(v), end(v), std::back_inserter(output), func);
//  return output;
//}

template <typename Container, typename Function>
static Container filter(const Container& source, const Function & func)
{
  Container output;
  std::copy_if(source.begin(), source.end(), std::inserter(output, output.end()), func);
  return output;
}

//template<typename InputType, typename OutputType, typename Func>
//static OutputType accumulate(const std::vector<InputType> & v, Func func, OutputType initialValue = OutputType(0))
//{
//  return std::accumulate(begin(v), end(v), initialValue, func);
//}

template <
  typename InputType,
  typename OutputType = InputType,
  template <typename, typename...> class Container,
  typename... Args,
  typename Func = std::plus<InputType>,
  typename = typename std::enable_if<std::is_arithmetic<InputType>::value, InputType>::type
>
static OutputType accumulate(const Container<InputType, Args...>& container, Func func = Func())
{
    return std::accumulate(container.begin(), container.end(), OutputType(), func);
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static std::vector<T> normalize(const std::vector<T> & v)
{
  T total = accumulate<T, T>(v, [](T carry, T x) { return carry + x; });
  return transform<T, T>(v, [&](T x) { return x / total; });
}

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static T average(const Container<T, Args...> & v)
{
  return accumulate<T, T>(v, [](T carry, T x) { return carry + x; }) / v.size();
}

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static T median(const Container<T, Args...> & v)
{
  auto sorted = v;
  std::sort(begin(sorted), end(sorted));
  
  size_t size = v.size();
  
  if (size % 2 == 0) return (v[(size - 1) / 2] + v[size / 2]) / 2.0;
  else return v[size / 2];
}

template<typename T>
static std::vector<T> intersection(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::vector<T> output;
  std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(output));
  return output;
}

template<typename T>
static std::vector<T> difference(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> output;
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::set_difference(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(output));
  return output;
}

template<typename T>
static bool includes(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  return std::includes(A.begin(), A.end(), B.begin(), B.end());
}

enum CoordinateEdges {
  WRAP, CLAMP, CRASH
};

static unsigned int coordinateToIndex(glm::ivec3 coord, glm::ivec3 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x + modulo(coord.z, size.z) * size.x * size.y;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x + CLAMP(coord.z, 0, size.z - 1) * size.x * size.y;
  else return coord.x + coord.y * size.x + coord.z * size.x * size.y;
}
static unsigned int coordinateToIndex(glm::ivec2 coord, glm::ivec2 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x;
  else return coord.x + coord.y * size.x;
}

static glm::ivec3 indexToCoordinate(int index, glm::ivec3 size)
{
  if (index < 0 || index >= size.x * size.y * size.z) { index = CLAMP(index, 0, size.x * size.y * size.z - 1); }
  
  glm::ivec3 coord;
  coord.x = index % size.x;
  coord.y = (index / size.x) % size.y;
  coord.z = index / (size.x * size.y);
  
  return coord;
}

static glm::ivec2 indexToCoordinate(int index, glm::ivec2 size)
{
  if (index < 0 || index >= size.x * size.y) { index = CLAMP(index, 0, size.x * size.y - 1); }
  
  glm::ivec2 coord;
  coord.x = index % size.x;
  coord.y = index / size.x;
  
  return coord;
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

static std::vector<ofColor> fromCoolors(const std::string & URL)
{
  std::vector<std::string> hexCodes = ofSplitString(URL.substr(URL.find_last_of("/") + 1), "-");
  
  return Array::transform<ofColor>(hexCodes, &Color::hexToColor);
}

static ofFloatColor proceduralPalette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
  t = ofWrap(t, 0.0, 1.0);
  
  glm::vec3 color = a + b * cos(TWO_PI * (c * t + d));
  return ofFloatColor(color.r, color.g, color.b);
}

static ofFloatColor rainbowPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.33, 0.67)); }
static ofFloatColor metallicPalette(float t) { return proceduralPalette(t, glm::vec3(0.5), glm::vec3(0.5), glm::vec3(1.0), glm::vec3(0.0, 0.1, 0.2)); }

static ofMesh getGradientMesh(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0)
{
  ofMesh mesh;
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
  
  int steps = w / 10.0;
  
  for (int i = 0; i < steps; i++)
  {
    float t = i / (float)(steps - 1);
    
    ofColor c = ofxCortex::core::utils::Shaping::interpolate(colors, t);
    
    float x = t * w;
    mesh.addVertex(glm::vec3(x, 0, 0));
    mesh.addColor(c);
    mesh.addVertex(glm::vec3(x, h, 0));
    mesh.addColor(c);
  }
  
  return mesh;
}

static ofMesh getGradientMeshVertical(const std::vector<ofColor> & colors, float w = 1.0, float h = 1.0)
{
  ofMesh mesh;
  mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
  
  int steps = h / 10.0;
  
  for (int i = 0; i < steps; i++)
  {
    float t = i / (float)(steps - 1);
    
    ofColor c = ofxCortex::core::utils::Shaping::interpolate(colors, t);
    
    float y = t * h;
    mesh.addVertex(glm::vec3(0, y, 0));
    mesh.addColor(c);
    mesh.addVertex(glm::vec3(w, y, 0));
    mesh.addColor(c);
  }
  
  return mesh;
}

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


#pragma mark - Vector
namespace Vector {

inline static glm::vec2 random2D(float radius = 1.0) { return glm::circularRand(MAX(radius, std::numeric_limits<float>::epsilon())); }
inline static glm::vec3 random3D(float radius = 1.0) { return glm::sphericalRand(MAX(radius, std::numeric_limits<float>::epsilon())); }

inline static glm::vec3 to3D(const glm::vec2 & v, float z = 0.0f) { return { v.x, v.y, z }; }
static std::vector<glm::vec3> to3D(const std::vector<glm::vec2> & vec, float z = 0.0f)
{
  std::vector<glm::vec3> output;
  for (const glm::vec2 & p : vec) output.push_back({ p.x, p.y, z });
  return output;
}

template<typename T>
inline static void translate(std::vector<T> & v, const T & p) { std::for_each(v.begin(), v.end(), [&p](T & e) { e += p; }); }

inline static glm::vec2 fromRadians(float angle) { return { cosf(angle), sinf(angle) }; }
inline static glm::vec2 fromDegrees(float angle) { return fromRadians(ofDegToRad(angle)); }
inline static glm::vec2 fromAngle(float angle) { return fromRadians(ofDegToRad(angle)); }

inline static float toRadians(const glm::vec2 & v) { return glm::atan(v.y, v.x); }
inline static float toDegrees(const glm::vec2 & v) { return ofRadToDeg(toRadians(v)); }
inline static float toAngle(const glm::vec2 & v) { return toDegrees(v); }

static float angleBetween(const glm::vec2 & a, const glm::vec2 & b) { return std::atan2(b.y, b.x) - std::atan2(a.y, a.x); }
static float angleBetween(const glm::vec3 & a, const glm::vec3 & b) 
{
  const glm::vec3 u = glm::cross(a, b);
  
  return atan2(glm::length(u), glm::dot(a, b)) * ofSign(u.z || 1);
}

template<typename T> inline static void flipX(T & v) { v.x * -1.0f; }
template<typename T> inline static void flipY(T & v) { v.y * -1.0f; }

template<typename T> inline static T getFlippedY(const T & v) { return v * T(1, -1, 1); }

static glm::vec2 getRotatedAroundOrigin(const glm::vec2 & point, const glm::vec2 & origin, float radians)
{
  const float co = cos(radians);
  const float si = sin(radians);
  
  float nx = (co * (point.x - origin.x)) + (si * (point.y - origin.y)) + origin.x;
  float ny = (co * (point.y - origin.y)) - (si * (point.x - origin.x)) + origin.y;
  return { nx, ny };
}

inline static void rotateAroundOrigin(glm::vec2 & point, const glm::bvec2 & origin, float radians) { point = getRotatedAroundOrigin(point, origin, radians); }

inline static glm::vec2 getRotated(const glm::vec2 & v, float radians)
{
  float a = radians;
  return glm::vec2(v.x * cos(a) - v.y * sin(a), v.x * sin(a) + v.y * cos(a));
}

template<typename T> inline static void limit(T & v, float max) { v = glm::min(v, glm::normalize(v) * max); }
template<typename T> inline static T getLimited(const T & v, float max) { return glm::min(v, glm::normalize(v) * max); }

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
