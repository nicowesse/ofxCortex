#pragma once

#include "ofUtils.h"

namespace ofxCortex { namespace types {

class OutputValue {
protected:
  std::string value;
  std::string unit { "" };
  
public:
  OutputValue() : value("") {}
  OutputValue(const std::string & _value, const std::string & _unit) : value(_value), unit(_unit) {}
  template<typename T> OutputValue(const T & _value, const std::string & _unit) : OutputValue(ofToString(_value), _unit) {}
  
  OutputValue(const OutputValue& other) : value(other.value), unit(other.unit) {}
  
  // Copy assignment operator
  OutputValue& operator=(const OutputValue& other) {
    if (this != &other) {  // Check for self-assignment
      value = other.value;
      unit = other.unit;
    }
    return *this;
  }
  
  // Assignment operator for generic types
  template<typename T>
  OutputValue& operator=(const T& _value) {
    value = ofToString(_value);
    return *this;
  }
  
  bool operator!() const { return value.empty(); }
  
  // Conversion to string
  operator const std::string&() const { return value + " " + unit; }
  
  friend std::ostream& operator<<(std::ostream& os, const OutputValue& dbg) {
    os << dbg.value << " " << dbg.unit;
    return os;
  }
};

}}
