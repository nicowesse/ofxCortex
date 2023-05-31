#pragma once

#include "ofImage.h"

namespace ofxCortex { namespace core { namespace types {

class File
{
public:
  std::string path;
  
  File() {};
  
  File (const std::string & path ) : path(path) {}
  
  File(const File &other) : path(other.path) {}
  
  File& operator= (const File &other)
  {
    if (this != &other)
    {
      this->path = other.path;
    }
    return *this;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const File& file)
  {
    os << file.path;
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, File &file)
  {
    is >> file.path;
    
    return is;
  }
  
};

}}}
