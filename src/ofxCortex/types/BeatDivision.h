#pragma once

#include "ofxCortex/utils/NumberUtils.h"

namespace ofxCortex { namespace core { namespace types {

struct BeatDivision {
  using value_type = int; // Define value_type as int
  
  BeatDivision(int value = 2) : value(value) {};
  BeatDivision(const BeatDivision &other) : value(other.value) {}
  
  BeatDivision& operator=(const BeatDivision &other)
  {
    if (this != &other) this->value = other.value;
    return *this;
  }
  
  BeatDivision& operator=(int val) {
    value = val % 5;
    return *this;
  }
  
  operator int() const { return value; }
  
  BeatDivision& operator+=(const BeatDivision& other) {
    value = clamp(value + other.value);
    return *this;
  }
  
  BeatDivision& operator-=(const BeatDivision& other) {
    value = clamp(value - other.value);
    return *this;
  }
  
  BeatDivision& operator*=(const BeatDivision& other) {
    value = clamp(value * other.value);
    return *this;
  }
  
  BeatDivision& operator/=(const BeatDivision& other) {
    if (other.value != 0) {
      value = clamp(value / other.value);
    }
    return *this;
  }
  
  // Increment and decrement operators
  BeatDivision& operator++() {
    value = clamp(value + 1);
    return *this;
  }
  
  BeatDivision operator++(int) {
    BeatDivision temp(*this);
    ++(*this);
    return temp;
  }
  
  BeatDivision& operator--() {
    value = clamp(value - 1);
    return *this;
  }
  
  BeatDivision operator--(int) {
    BeatDivision temp(*this);
    --(*this);
    return temp;
  }
  
  friend std::ostream& operator<<(std::ostream& os, const BeatDivision& division)
  {
    int wrapped = utils::modulo(division, 6);
    if (wrapped == 0) os << "1/1"; // Bar
    else if (wrapped == 1) os << "1/2"; // Half
    else if (wrapped == 2) os << "1/4"; // Quarter/Beat
    else if (wrapped == 3) os << "1/8"; // Eight
    else if (wrapped == 4) os << "1/16"; // Sixteenth
    else if (wrapped == 5) os << "1/32"; // Sixteenth
    
    return os;
  }
  
  friend std::istream& operator>> (std::istream &is, BeatDivision& division)
  {
    std::string incoming(std::istreambuf_iterator<char>(is), {});
    
    if (incoming == "1/1") division = 0;
    else if (incoming == "1/2") division = 1;
    else if (incoming == "1/4") division = 2;
    else if (incoming == "1/8") division = 3;
    else if (incoming == "1/16") division = 4;
    else if (incoming == "1/32") division = 5;
    
    return is;
  }
  
  friend bool operator==(const BeatDivision& beat, int val) { return beat.value == val; }
  friend bool operator==(int val, const BeatDivision& beat) { return val == beat.value; }
  
protected:
  int value;
  
  static int clamp(int val) { return std::max(0, std::min(5, val)); }
};

}}}

namespace std {
template<>
class numeric_limits<ofxCortex::core::types::BeatDivision> {
public:
  static constexpr bool is_specialized = true;
  static ofxCortex::core::types::BeatDivision lowest() noexcept { return ofxCortex::core::types::BeatDivision(0); }
  static ofxCortex::core::types::BeatDivision min() noexcept { return ofxCortex::core::types::BeatDivision(0); }
  static ofxCortex::core::types::BeatDivision max() noexcept { return ofxCortex::core::types::BeatDivision(5); }
};

template<> struct is_arithmetic<ofxCortex::core::types::BeatDivision> : std::true_type {};
}
