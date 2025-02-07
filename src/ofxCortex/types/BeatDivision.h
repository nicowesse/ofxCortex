#pragma once

#include "ofxCortex/utils/NumberUtils.h"
#include "ofxCortex/utils/AudioUtils.h"
#include "ofxCortex/types/Select.h"

namespace ofxCortex { namespace core { namespace types {

struct BeatDivision : public Select<int> {
  
  BeatDivision(int selected = 2) : Select<int>(std::vector<std::pair<int, std::string>>{
    { 0, "1/1" },
    { 1, "1/2" },
    { 2, "1/4" },
    { 3, "1/8" },
    { 4, "1/16" },
    { 5, "1/32" }
  }, selected) {};
  
  BeatDivision& operator=(const BeatDivision &other)
  {
    if (this != &other) this->selectedIndex = other.selectedIndex;
    return *this;
  }
  
  BeatDivision& operator=(int val) { selectedIndex = val % 5; return *this; }
  operator int() const { return selectedIndex; }
  
  BeatDivision& operator+=(const BeatDivision& other) { selectedIndex = clamp(selectedIndex + other.selectedIndex); return *this; }
  BeatDivision& operator-=(const BeatDivision& other) { selectedIndex = clamp(selectedIndex - other.selectedIndex); return *this; }
  BeatDivision& operator*=(const BeatDivision& other) { selectedIndex = clamp(selectedIndex * other.selectedIndex); return *this; }
  
  BeatDivision& operator/=(const BeatDivision& other) {
    if (other.selectedIndex != 0) {
      selectedIndex = clamp(selectedIndex / other.selectedIndex);
    }
    return *this;
  }
  
  // Increment and decrement operators
  BeatDivision& operator++() { selectedIndex = (selectedIndex + 1) % 5; return *this; }
  
  BeatDivision operator++(int) {
    BeatDivision temp(*this);
    ++(*this);
    return temp;
  }
  
  BeatDivision& operator--() { selectedIndex = clamp(selectedIndex - 1); return *this; }
  
  BeatDivision operator--(int) {
    BeatDivision temp(*this);
    --(*this);
    return temp;
  }
  
  friend bool operator==(const BeatDivision& beat, int val) { return beat.selectedIndex == val; }
  friend bool operator==(int val, const BeatDivision& beat) { return val == beat.selectedIndex; }
  
protected:
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
