#pragma once

#include "ofUtils.h"

namespace ofxCortex { namespace types {

class DisplayValue {
protected:
  function<std::string()> func;
  std::string unit { "" };
  
public:
  DisplayValue() = default;
  DisplayValue(function<std::string()> _func, const std::string & _unit = "") : func(_func), unit(_unit) {}
  DisplayValue(const DisplayValue& other) : func(other.func), unit(other.unit) {}
  
  // Copy assignment operator
  DisplayValue& operator=(const DisplayValue& other) {
    if (this != &other) {  // Check for self-assignment
      func = other.func;
      unit = other.unit;
    }
    return *this;
  }
  
  // Conversion to string
  operator const std::string&() const { return this->func() + " " + this->unit; }
  
  friend std::ostream& operator<<(std::ostream& os, const DisplayValue& dbg) {
    os << dbg.func() << " " << dbg.unit;
    return os;
  }
};

}}
