#pragma once

#include "ofImage.h"

namespace ofxCortex { namespace core { namespace types {

class Image
{
public:
  ofImage image;
  std::string path;
  ofRectangle rect;
  
  Image() {};
  
  Image (const std::string & path ) {
    this->load(path);
  }
  
  Image(const ofImage & image) : image(image), rect(0, 0, image.getWidth(), image.getHeight()), path("") {};
  
  Image(const Image &other) : image(other.image), path(other.path), rect(other.rect) {
//    this->load(other.path);
  }
  
  Image& operator= (const Image &other)
  {
    if (this != &other)
    {
      this->image = other.image;
      this->path = other.path;
      this->rect = other.rect;
    }
    return *this;
  }
  
  void load(const std::string & path)
  {
    this->path = path;
    if (image.load(path))
    {
      ofLogVerbose("types::Image") << "✅ Image loaded from '" << path << "'";
      this->rect = ofRectangle(0, 0, image.getWidth(), image.getHeight());
      this->image.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    }
    else
    {
      ofLogWarning("types::Image") << "⚠️ Image not loaded! Is the path correct? Current one is '" << path << "'";
    }
  }
  
  const ofImage & getImage() const {
    return image;
  }
  
  glm::vec2 getSize() const {
    return glm::vec2(rect.width, rect.height);
  }
  
  friend std::ostream& operator<<(std::ostream& os, const Image& image)
  {
    os << image.path;
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, Image &image)
  {
    is >> image.path;
    image.load(image.path);
    
    return is;
  }
  
};

}}}
