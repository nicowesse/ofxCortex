#pragma once

#include "ofParameter.h"

namespace ofxCortex::core::units {
  constexpr char px[] = "px";
  constexpr char cm[] = "cm";
}

namespace ofxCortex::core::types {

template<typename ParameterType, char const* unit = ofxCortex::core::units::px>
class Parameter : public ofAbstractParameter {
public:
  Parameter() {};
  Parameter(const ofParameter<ParameterType> & v, const std::string & unit) : param(v), unit(unit) {};
  Parameter(const ParameterType & v, const std::string & unit) : param(v), unit(unit) {};
  Parameter(const std::string& name, const ParameterType & v) : param(name, v) {};
  Parameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) : param(name, v, min, max) {
    std::cout << "Unit Parameter with unit = " << unt << std::endl;
  };
  
  std::string type() const override { return "Parameter"; };
  
  const ParameterType & get() const { return param.get(); };
  operator const ParameterType & () const { return &param; };
  
  void setName(const std::string & name) override { param.setName(name); };
  std::string getName() const override { return param.getName(); };
  
  ParameterType getMin() const { return param.getMin(); };
  ParameterType getMax() const { return param.getMax(); };
  
  //  friend std::ostream& operator<<(std::ostream& os, const Parameter<ParameterType>& p)
  //  {
  //    os << "Hello";
  //    return os;
  //  }
  //
  //  friend std::istream& operator>>(std::istream& is, ofAbstractParameter& p)
  //  {
  //
  //  }
  
  std::string toString() const override {
    std::cout << "Parameter::toString()" << std::endl;
    return param.toString() + " " + unt;
  };
  inline void fromString(const std::string & name) {
    std::cout << "Parameter::toString() " << name << std::endl;
    param.fromString(name);
  };
  
  template<class ListenerClass, typename ListenerMethod>
  inline void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    param.addListener(listener, method, prio);
  }
  
  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    param.removeListener(listener, method, prio);
  }
  
  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return param.newListener(args...);
  }
  
  template<typename... Args>
  void ownListener(Args...args) { param.ownListener(args...); }
  
  void enableEvents() { param.enableEvents(); };
  void disableEvents() { param.disableEvents(); };
  bool isSerializable() const override { return param.isSerializable(); };
  bool isReadOnly() const override { return param.isReadOnly(); };
  std::string valueType() const override { return param.valueType(); };
  
  inline void makeReferenceTo(ofParameter<ParameterType> & mom) { param.makeReferenceTo(mom); };
  
  inline ofParameter<ParameterType> & operator=(const ofParameter<ParameterType> & v) { return param = v; };
  inline const ParameterType & operator=(const ParameterType & v) { return param = v; };
  
  ParameterType operator++(int v) { return param++; };
  inline ofParameter<ParameterType> & operator++() { return param++; };
  
  inline ParameterType operator--(int v) { return param--; };
  inline ofParameter<ParameterType> & operator--() { return param--; };
  
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator+=(const OtherType & v) { return param += v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator-=(const OtherType & v) { return param -= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator*=(const OtherType & v) { return param *= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator/=(const OtherType & v) { return param /= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator%=(const OtherType & v) { return param %= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator&=(const OtherType & v) { return param &= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator|=(const OtherType & v) { return param |= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator^=(const OtherType & v) { return param ^= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator<<=(const OtherType & v) { return param <<= v; };
  template<typename OtherType>
  inline ofParameter<ParameterType> & operator>>=(const OtherType & v) { return param >>= v; };
  
  
  inline ofParameter<ParameterType> & set(const ParameterType & v) { return param.set(v); };
  inline ofParameter<ParameterType> & set(const std::string& name, const ParameterType & v) { return param.set(name, v); };
  inline ofParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) { return param.set(name, v, min, max); };
  
  inline ofParameter<ParameterType> & setWithoutEventNotifications(const ParameterType & v) { return param.setWithoutEventNotifications(v); };
  
  inline void setMin(const ParameterType & min) { param.setMin(min); };
  inline void setMax(const ParameterType & max) { param.setMax(max); };
  
  inline void setSerializable(bool serializable) { param.setSerializable(serializable); };
  inline std::shared_ptr<ofAbstractParameter> newReference() const { return param.newReference(); };
  
  inline void setParent(ofParameterGroup & _parent) { param.setParent(_parent); };
  
  inline const ofParameterGroup getFirstParent() const { return param.getFirstParent(); }
  
  inline std::size_t getNumListeners() const { return param.getNumListeners(); };
  inline const void* getInternalObject() const { return param.getInternalObject(); };
  
protected:
  ofParameter<ParameterType> param;
  std::string unit;
};

}
