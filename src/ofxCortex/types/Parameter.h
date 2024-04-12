#pragma once

#include "ofEvents.h"
#include "ofPoint.h"
#include "ofRectangle.h"
#include "ofLog.h"
#include "ofConstants.h"
#include "ofColor.h"
#include "ofParameter.h"
#include "ofJson.h"

#include <map>

namespace ofxCortex::core::units {

constexpr auto px = " px";
constexpr auto pxs = " px/s";

constexpr auto cm = " cm";
constexpr auto mm = " mm";

constexpr auto ms = " ms";
constexpr auto seconds = " sec";

constexpr auto degrees = "°";
constexpr auto radians = " rad";

constexpr auto percentage = " %";

}

template<typename ParameterType>
class CustomParameter;

class CustomAbstractParameter : public ofAbstractParameter {
public:
  CustomAbstractParameter(){};
  virtual ~CustomAbstractParameter(){};
  
  virtual std::string getName() const = 0;
  virtual void setName(const std::string & name) = 0;
  virtual std::string toString() const = 0;
  virtual void fromString(const std::string & str) = 0;
  
  virtual std::string valueType() const = 0;
  
  virtual void setParent(ofParameterGroup & _parent) = 0;
  
  template<typename ParameterType>
  CustomParameter<ParameterType> & cast(){
    return static_cast<CustomParameter<ParameterType> &>(*this);
  }
  
  template<typename ParameterType>
  const CustomParameter<ParameterType> & cast() const{
    return static_cast<const CustomParameter<ParameterType> &>(*this);
  }
  
  virtual bool isSerializable() const = 0;
  virtual bool isReadOnly() const = 0;
  virtual std::shared_ptr<ofAbstractParameter> newReference() const = 0;
  
protected:
  virtual const ofParameterGroup getFirstParent() const = 0;
  virtual void setSerializable(bool serializable)=0;
  virtual const void* getInternalObject() const = 0;
  
private:
  
};

namespace ofxCortex {

template<typename ParameterType>
class MetaParameter : public ofAbstractParameter {
public:
  MetaParameter() : parameter() {};
  MetaParameter(const ofParameter<ParameterType> & v) : parameter(v) {};
  MetaParameter(const MetaParameter<ParameterType> & other) : parameter(other.parameter), metadata(other.metadata) {};
  MetaParameter(const ParameterType & v) : parameter(v) { };
  MetaParameter(const std::string& name, const ParameterType & v) : parameter(name, v) {};
  MetaParameter(const std::string& name, const ParameterType & v, const std::unordered_map<std::string, std::any> & meta) : parameter(name, v), metadata(meta) {};
  MetaParameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) : parameter(name, v, min, max) {};
  MetaParameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::unordered_map<std::string, std::any> & meta) : parameter(name, v, min, max), metadata(metadata) {};
  virtual ~MetaParameter() {}
  
  const ParameterType & get() const { return parameter.get(); };
  const ParameterType * operator->() const { return &parameter.get(); };
  operator const ParameterType & () const { return parameter.get(); };
  
  ofParameter<ParameterType> & getParameter() { return parameter; }
  explicit operator ofParameter<ParameterType>() const { return parameter; }
  explicit operator ofParameter<ParameterType>&() const { return parameter; }
  operator const ofParameter<ParameterType>&() const { return parameter; }
  
  virtual std::string getName() const override { return parameter.getName(); }
  virtual void setName(const std::string & name) override { parameter.setName(name); }
  virtual std::string toString() const override { return parameter.toString(); }
  virtual void fromString(const std::string & str) override { parameter.fromString(str); }
  
  virtual std::string type() const override { return typeid(*this).name(); }
  
  ParameterType getMin() const { return parameter.getMin(); };
  ParameterType getMax() const { return parameter.getMax(); };
  ParameterType getInit() const { return parameter.getInit(); };
  void reInit() { parameter.reInit(); };
  
  template<class ListenerClass, typename ListenerMethod>
  void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.addListener(listener, method, prio);
  }
  
  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.removeListener(listener, method, prio);
  }
  
  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return parameter.newListener(args...);
  }
  
  void enableEvents() { parameter.enableEvents(); };
  void disableEvents() { parameter.disableEvents(); };
  virtual bool isSerializable() const override { return parameter.isSerializable(); };
  virtual bool isReadOnly() const override { return parameter.isReadOnly(); };
  virtual std::string valueType() const override { return parameter.valueType(); };
  
  void makeReferenceTo(MetaParameter<ParameterType> mom) { parameter.makeReferenceTo(mom.parameter); };
  void makeReferenceTo(ofParameter<ParameterType> mom) { parameter.makeReferenceTo(mom); };
  
  MetaParameter<ParameterType> & operator=(const MetaParameter<ParameterType> & other) {
    parameter = other.parameter;
    metadata= other.metadata;
    
    return *this;
  };
  
  MetaParameter<ParameterType> & operator=(const ofParameter<ParameterType> & v) { parameter = v; return *this; };
  const ParameterType & operator=(const ParameterType & v) { parameter = v; return v; };
  
  ParameterType operator++(int v) { return parameter++; }
  MetaParameter<ParameterType> & operator++() { return ++parameter; }
  
  ParameterType operator--(int v) { return parameter--; }
  MetaParameter<ParameterType> & operator--() { return --parameter; }
  
  template<typename OtherType>
  MetaParameter<ParameterType> & operator+=(const OtherType & v) {
    parameter += v;
    return *this;
  }
  template<typename OtherType> MetaParameter<ParameterType> & operator-=(const OtherType & v) { parameter -= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator*=(const OtherType & v) { parameter *= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator/=(const OtherType & v) { parameter /= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator%=(const OtherType & v) { parameter %= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator&=(const OtherType & v) { parameter &= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator|=(const OtherType & v) { parameter |= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator^=(const OtherType & v) { parameter ^= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator<<=(const OtherType & v) { parameter <<= v; return *this; }
  template<typename OtherType> MetaParameter<ParameterType> & operator>>=(const OtherType & v) { parameter >>= v; return *this; }
  
  
  MetaParameter<ParameterType> & set(const ParameterType & v) { parameter.set(v); return *this; }
  MetaParameter<ParameterType> & set(const std::string& name, const ParameterType & v) { parameter.set(name, v); return *this; }
  MetaParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const std::string & unit) { this->unit = unit; parameter.set(name, v); return *this; }
  MetaParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) { parameter.set(name, v, min, max); return *this; }
  MetaParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit) { this->unit = unit; parameter.set(name, v, min, max); return *this; }
  
  MetaParameter<ParameterType> & setWithoutEventNotifications(const ParameterType & v) { return parameter.setWithoutEventNotifications(v); };
  
  void setMin(const ParameterType & min) { parameter.setMin(min); }
  void setMax(const ParameterType & max) { parameter.setMax(max); }
  void setInit(const ParameterType & init) { parameter.setInit(init); }
  
  virtual void setSerializable(bool serializable) override { parameter.setSerializable(serializable); }
  virtual std::shared_ptr<ofAbstractParameter> newReference() const override { return std::make_shared<MetaParameter<ParameterType>>(*this); }
  
  virtual void setParent(ofParameterGroup & _parent) override { parameter.setParent(_parent); };
  
  virtual const ofParameterGroup getFirstParent() const override {
    return parameter.getFirstParent();
  }
  
  size_t getNumListeners() const { return parameter.getNumListeners(); }
  virtual const void* getInternalObject() const override { return parameter.getInternalObject(); }
  
  template<typename T>
  friend class ofParameter;
  friend class ofParameterGroup;
  
protected:
  ofParameter<ParameterType> parameter;
  std::unordered_map<std::string, std::any> metadata;
  
};




template<typename ParameterType>
class UnitParameter : public ofAbstractParameter {
public:
  UnitParameter() : parameter(), unit("") {};
  UnitParameter(const ofParameter<ParameterType> & v) : parameter(v), unit("") {};
  UnitParameter(const UnitParameter<ParameterType> & other) : parameter(other.parameter), unit(other.unit) {};
  UnitParameter(const ParameterType & v) : parameter(v), unit("") { };
  UnitParameter(const std::string& name, const ParameterType & v) : parameter(name, v), unit("") {};
  UnitParameter(const std::string& name, const ParameterType & v, const std::string & unit) : parameter(name, v), unit(unit) {};
  UnitParameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) : parameter(name, v, min, max), unit("") {};
  UnitParameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit) : parameter(name, v, min, max), unit(unit) {};
  virtual ~UnitParameter() {}
  
  const ParameterType & get() const { return parameter.get(); };
  const ParameterType * operator->() const { return &parameter.get(); };
  operator const ParameterType & () const { return parameter.get(); };
  
  ofParameter<ParameterType> & getParameter() { return parameter; }
  explicit operator ofParameter<ParameterType>() const { return parameter; }
  explicit operator ofParameter<ParameterType>&() const { return parameter; }
  operator const ofParameter<ParameterType>&() const { return parameter; }
  
  virtual std::string getName() const override { return parameter.getName(); }
  virtual void setName(const std::string & name) override { parameter.setName(name); }
  virtual std::string toString() const override { return parameter.toString(); }
  virtual void fromString(const std::string & str) override { parameter.fromString(str); }
  
  virtual std::string type() const override { return typeid(*this).name(); }
  
  ParameterType getMin() const { return parameter.getMin(); };
  ParameterType getMax() const { return parameter.getMax(); };
  ParameterType getInit() const { return parameter.getInit(); };
  void reInit() { parameter.reInit(); };
  
  std::string getUnit() const { return unit; }
  void setUnit(const std::string & _unit) { this->unit = _unit; }
  
  template<class ListenerClass, typename ListenerMethod>
  void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.addListener(listener, method, prio);
  }
  
  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.removeListener(listener, method, prio);
  }
  
  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return parameter.newListener(args...);
  }
  
  void enableEvents() { parameter.enableEvents(); };
  void disableEvents() { parameter.disableEvents(); };
  virtual bool isSerializable() const override { return parameter.isSerializable(); };
  virtual bool isReadOnly() const override { return parameter.isReadOnly(); };
  virtual std::string valueType() const override { return parameter.valueType(); };
  
  void makeReferenceTo(UnitParameter<ParameterType> mom) { parameter.makeReferenceTo(mom.parameter); };
  void makeReferenceTo(ofParameter<ParameterType> mom) { parameter.makeReferenceTo(mom); };
  
  UnitParameter<ParameterType> & operator=(const UnitParameter<ParameterType> & other) {
    parameter = other.parameter;
    unit = other.unit;
    return *this;
  };
  
  UnitParameter<ParameterType> & operator=(const ofParameter<ParameterType> & v) { parameter = v; return *this; };
  const ParameterType & operator=(const ParameterType & v) { parameter = v; return v; };
  
  ParameterType operator++(int v) { return parameter++; }
  
  UnitParameter<ParameterType> & operator++() { return ++parameter; }
  
  ParameterType operator--(int v) { return parameter--; }
  UnitParameter<ParameterType> & operator--() { return --parameter; }
  
  template<typename OtherType> UnitParameter<ParameterType> & operator+=(const OtherType & v) { parameter += v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator-=(const OtherType & v) { parameter -= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator*=(const OtherType & v) { parameter *= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator/=(const OtherType & v) { parameter /= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator%=(const OtherType & v) { parameter %= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator&=(const OtherType & v) { parameter &= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator|=(const OtherType & v) { parameter |= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator^=(const OtherType & v) { parameter ^= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator<<=(const OtherType & v) { parameter <<= v; return *this; }
  template<typename OtherType> UnitParameter<ParameterType> & operator>>=(const OtherType & v) { parameter >>= v; return *this; }
  
  
  UnitParameter<ParameterType> & set(const ParameterType & v) { parameter.set(v); return *this; }
  UnitParameter<ParameterType> & set(const std::string& name, const ParameterType & v) { parameter.set(name, v); return *this; }
  UnitParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const std::string & unit) { this->unit = unit; parameter.set(name, v); return *this; }
  UnitParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max) { parameter.set(name, v, min, max); return *this; }
  UnitParameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit) { this->unit = unit; parameter.set(name, v, min, max); return *this; }
  
  UnitParameter<ParameterType> & setWithoutEventNotifications(const ParameterType & v) { return parameter.setWithoutEventNotifications(v); };
  
  void setMin(const ParameterType & min) { parameter.setMin(min); }
  void setMax(const ParameterType & max) { parameter.setMax(max); }
  void setInit(const ParameterType & init) { parameter.setInit(init); }
  
  virtual void setSerializable(bool serializable) override { parameter.setSerializable(serializable); }
  virtual std::shared_ptr<ofAbstractParameter> newReference() const override { return std::make_shared<UnitParameter<ParameterType>>(*this); }
  
  virtual void setParent(ofParameterGroup & _parent) override { parameter.setParent(_parent); };
  
  virtual const ofParameterGroup getFirstParent() const override { return parameter.getFirstParent(); }
  
  size_t getNumListeners() const { return parameter.getNumListeners(); }
  virtual const void* getInternalObject() const override { return parameter.getInternalObject(); }
  
  template<typename T>
  friend class ofParameter;
  friend class ofParameterGroup;
  
protected:
  ofParameter<ParameterType> parameter;
  std::string unit;
  
};

template<>
class UnitParameter<void> : public ofAbstractParameter {
public:
  UnitParameter() : parameter(), unit("") {};
  UnitParameter(const std::string & name) : parameter(name), unit("") {};
  UnitParameter(const ofParameter<void> & v) : parameter(v), unit("") {};
  virtual ~UnitParameter() {}
  
  ofParameter<void> & getParameter() { return parameter; }
  explicit operator ofParameter<void>() const { return parameter; }
  operator const ofParameter<void>&() const { return parameter; }
  
  virtual std::string getName() const override { return parameter.getName(); }
  virtual void setName(const std::string & name) override { parameter.setName(name); }
  virtual std::string toString() const override { return parameter.toString(); }
  virtual void fromString(const std::string & str) override { parameter.fromString(str); }
  
  virtual std::string type() const override { return typeid(*this).name(); }
  
  std::string getUnit() const { return unit; }
  void setUnit(const std::string & _unit) { this->unit = _unit; }
  
  template<class ListenerClass, typename ListenerMethod>
  void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.addListener(listener, method, prio);
  }
  
  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    parameter.removeListener(listener, method, prio);
  }
  
  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return parameter.newListener(args...);
  }
  
  void trigger() { parameter.trigger(); }
  void trigger(const void * sender) { parameter.trigger(sender); }
  
  void enableEvents() { parameter.enableEvents(); };
  void disableEvents() { parameter.disableEvents(); };
  virtual bool isSerializable() const override { return parameter.isSerializable(); };
  virtual bool isReadOnly() const override { return parameter.isReadOnly(); };
  virtual std::string valueType() const override { return parameter.valueType(); };
  
  void makeReferenceTo(UnitParameter<void> mom) { parameter.makeReferenceTo(mom.parameter); };
  void makeReferenceTo(ofParameter<void> mom) { parameter.makeReferenceTo(mom); };
  
  UnitParameter<void> & operator=(const UnitParameter<void> & v) {
    parameter = v.parameter;
    return *this;
  };
  
  UnitParameter<void> & operator=(const ofParameter<void> & v) { parameter = v; return *this; };
  
  virtual void setSerializable(bool serializable) override { parameter.setSerializable(serializable); }
  virtual std::shared_ptr<ofAbstractParameter> newReference() const override { return std::make_shared<UnitParameter<void>>(*this); }
  
  virtual void setParent(ofParameterGroup & _parent) override { parameter.setParent(_parent); };
  
  virtual const ofParameterGroup getFirstParent() const override {
    return parameter.getFirstParent();
  }
  
  size_t getNumListeners() const { return parameter.getNumListeners(); }
  virtual const void* getInternalObject() const override { return parameter.getInternalObject(); }
  
  template<typename T>
  friend class ofParameter;
  friend class ofParameterGroup;
  
protected:
  ofParameter<void> parameter;
  std::string unit;
  
};

}
