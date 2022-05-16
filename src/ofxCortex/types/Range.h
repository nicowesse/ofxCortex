#pragma once

namespace ofxCortex { namespace core { namespace types {

class Range
{
public:
  float min;
  float from;
  float to;
  float max;
  
  Range (float from, float to, float min, float max) : min(MIN(min, from)), from(from), to(to), max(MAX(max, to)) {}
  Range (float from, float to) : Range(from, from, to, to) {}
  Range () : Range(0, 1, 0, 1) {}
  
  Range(const Range &other) : min(other.min), from(other.from), to(other.to), max(other.max) {}
  
  Range& operator= (const Range &other)
  {
    if (this != &other)
    {
      this->min = other.min;
      this->from = other.from;
      this->to = other.to;
      this->max = other.max;
    }
    return *this;
  }
  
  Range& operator += (const float& rhs)
  {
    if (this->from + rhs >= this->min && this->to + rhs <= this->max)
    {
      this->from += rhs;
      this->to += rhs;
    }
    
    return *this;
  }
  
  Range& operator -= (const float& rhs)
  {
    if (this->from - rhs >= this->min && this->to - rhs <= this->max)
    {
      this->from -= rhs;
      this->to -= rhs;
    }
    
    return *this;
  }
  
  inline float getNormalizedFrom() const { return ofMap(from, min, max, 0.0f, 1.0f, true); }
  inline float getNormalizedTo() const { return ofMap(to, min, max, 0.0, 1.0f, true); }
  
  float getRandom() const { return ofRandom(this->from, this->to); }
  float map(float value, float lower = 0.0f, float upper = 1.0f) const { return ofMap(value, lower, upper, this->from, this->to, true); }
  
  void print() const { cout << "ofxCortex::types::Range" << " [" << min << "] " << ofToString(from, 2) << " <-> " << ofToString(to, 2) << " [" << max << "]" << endl; }
  
  friend ostream& operator<<(ostream& os, const Range& range)
  {
    os << range.min << "," << range.from << "," << range.to << "," << range.max;
    return os;
  }
  
  friend istream& operator>> (istream &is, Range &range)
  {
    is >> range.min;
    is.ignore(numeric_limits<streamsize>::max(), ',');
    is >> range.from;
    is.ignore(numeric_limits<streamsize>::max(), ',');
    is >> range.to;
    is.ignore(numeric_limits<streamsize>::max(), ',');
    is >> range.max;
    
    return is;
  }
  
};

}}}
