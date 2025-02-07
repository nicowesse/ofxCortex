#pragma once

namespace ofxCortex { namespace types {

enum class Status {
  OFF = 0,
  ON,
  DISCONNECTED,
  CONNECTED,
  CONNECTING,
  RECEIVING, SENDING,
};

inline std::ostream& operator<<(std::ostream& os, const Status& status) {
    os << static_cast<int>(status);
    return os;
} 

// Input streaming
inline std::istream& operator>>(std::istream& is, Status& status) {
  int value;
  is >> value;
  status = static_cast<Status>(value);
  return is;
}
  
}}
