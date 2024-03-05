#include "Helpers.h"

namespace ofxCortex { namespace core { namespace utils {

bool rayTriangleIntersection(const ofMeshFace & face, const glm::vec3 & O, const glm::vec3 & D, glm::vec3 & R)
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

bool isInsideMesh(const glm::vec3 & p, const ofMesh & mesh)
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

ofxCortex::core::types::Box getMeshBoundingBox(const ofMesh & mesh)
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

ofRectangle getBitmapStringBoundingBox(const std::string & text)
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

void drawTexCoordRectangle(float x, float y, float w, float h)
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

namespace String {

std::string toTitleCase(const std::string& str) {
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

namespace Color {

ofColor hexToColor(const std::string & hex)
{
  unsigned int hexValue;
  std::istringstream iss(hex);
  iss >> std::hex >> hexValue;
  
  int r = ((hexValue >> 16) & 0xFF);  // Extract the RR byte
  int g = ((hexValue >> 8) & 0xFF);   // Extract the GG byte
  int b = ((hexValue) & 0xFF);
  
  return ofColor(r, g, b);
}

ofMesh getGradientMesh(const std::vector<ofColor> & colors, float w, float h)
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

ofMesh getGradientMeshVertical(const std::vector<ofColor> & colors, float w, float h)
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

namespace Vector {

glm::vec2 getRotatedAroundOrigin(const glm::vec2 & point, const glm::vec2 & origin, float radians)
{
  const float co = cos(radians);
  const float si = sin(radians);
  
  float nx = (co * (point.x - origin.x)) + (si * (point.y - origin.y)) + origin.x;
  float ny = (co * (point.y - origin.y)) - (si * (point.x - origin.x)) + origin.y;
  return { nx, ny };
}

}

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
