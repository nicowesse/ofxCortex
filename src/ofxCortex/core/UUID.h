#pragma once

namespace ofxCortex { namespace core {

class UUID {
public:
  UUID();
  UUID(uint64_t uuid);
  UUID(const UUID&) = default;
  
  operator uint64_t() const { return _uuid; }
  operator std::string() const { return std::to_string(_uuid); }
  bool operator== (UUID const& rhs) const { return this->_uuid == rhs._uuid; }
  
  std::string toString() const { return std::to_string(_uuid); }
  
  friend std::ostream &operator<< (std::ostream & output, const UUID & uuid ) {
    output << uuid.toString();
    return output;
  }
  
private:
  uint64_t _uuid;
};

}}

namespace std {

template<>
struct hash<ofxCortex::core::UUID>
{
  std::size_t operator() (const ofxCortex::core::UUID & uuid) const
  {
    return hash<uint64_t>()((uint64_t) uuid);
  }
};
}
