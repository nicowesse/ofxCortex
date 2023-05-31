#pragma once

namespace ofxCortex { namespace core { namespace types {

class Spacer
{
public:
  int rows;
  
  Spacer(int rows = 1) : rows(rows) {};
  
  Spacer(const Spacer &other) : rows(other.rows) {}
  
  Spacer& operator= (const Spacer &other)
  {
    if (this != &other)
    {
      this->rows = other.rows;
    }
    return *this;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const Spacer& spacer)
  {
    os << spacer.rows;
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, Spacer &spacer)
  {
    is >> spacer.rows;
    
    return is;
  }
};
  
}}}
