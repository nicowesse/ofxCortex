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

constexpr auto px = "px";
constexpr auto pxs = "px/s";
constexpr auto cm = "cm";
constexpr auto mm = "mm";

}

namespace ofxCortex {

template<typename ParameterType>
class Parameter;

class Group;

class AbstractParameter{
public:
  virtual ~AbstractParameter(){}
  virtual std::string getName() const = 0;
  virtual void setName(const std::string & name) = 0;
  virtual std::string toString() const = 0;
  virtual void fromString(const std::string & str) = 0;

  virtual std::string type() const;
  virtual std::string getEscapedName() const;
  virtual std::string valueType() const = 0;

  virtual void setParent(Group & _parent) = 0;
  std::vector<std::string> getGroupHierarchyNames() const;

  template<typename ParameterType>
  Parameter<ParameterType> & cast(){
    return static_cast<Parameter<ParameterType> &>(*this);
  }

  template<typename ParameterType>
  const Parameter<ParameterType> & cast() const{
    return static_cast<const Parameter<ParameterType> &>(*this);
  }

  Group & castGroup();
  const Group & castGroup() const;

  friend std::ostream& operator<<(std::ostream& os, const AbstractParameter& p);
  friend std::istream& operator>>(std::istream& is, AbstractParameter& p);

  virtual bool isSerializable() const = 0;
  virtual bool isReadOnly() const = 0;
  virtual std::shared_ptr<AbstractParameter> newReference() const = 0;

  virtual bool isReferenceTo(const AbstractParameter& other) const;

protected:
  virtual const Group getFirstParent() const = 0;
  virtual void setSerializable(bool serializable)=0;
  virtual std::string escape(const std::string& str) const;
  virtual const void* getInternalObject() const = 0;
};


class Group: public AbstractParameter {
public:
  Group();

  template<typename ...Args>
  Group(const std::string & name)
  :obj(std::make_shared<Value>()){
    setName(name);
  }

  template<typename ...Args>
  Group(const std::string & name, Args&... p)
  :obj(std::make_shared<Value>()){
    add(p...);
    setName(name);
  }

  template<typename ...Args>
  void add(AbstractParameter & p, Args&... parameters){
    add(p);
    add(parameters...);
  }

  void add(AbstractParameter & param);
  std::string valueType() const;

  void remove(AbstractParameter & param);
  void remove(std::size_t index);
  void remove(const std::string& name);

  void clear();
  
  const Group & getGroup(const std::string& name) const;
  const Group & getGroup(std::size_t pos) const;
  Group & getGroup(const std::string& name);
  Group & getGroup(std::size_t pos);

  const AbstractParameter & get(const std::string& name) const;
  const AbstractParameter & get(std::size_t pos) const;

  const AbstractParameter & operator[](const std::string& name) const;
  const AbstractParameter & operator[](std::size_t pos) const;

  AbstractParameter & get(const std::string& name);
  AbstractParameter & get(std::size_t pos);

  AbstractParameter & operator[](const std::string& name);
  AbstractParameter & operator[](std::size_t pos);

  template<typename ParameterType>
  const Parameter<ParameterType> & get(const std::string& name) const;

  template<typename ParameterType>
  const Parameter<ParameterType> & get(std::size_t pos) const;

  template<typename ParameterType>
  Parameter<ParameterType> & get(const std::string& name);

  template<typename ParameterType>
  Parameter<ParameterType> & get(std::size_t pos);

  std::size_t size() const;
  std::string getName(std::size_t position) const;
  std::string getType(std::size_t position) const;
  bool getIsReadOnly(int position) const;
  int getPosition(const std::string& name) const;

  friend std::ostream& operator<<(std::ostream& os, const Group& group);

  std::string getName() const;
  void setName(const std::string& name);
  std::string getEscapedName() const;
  std::string toString() const;
  void fromString(const std::string& name);

  bool contains(const std::string& name) const;

  AbstractParameter & back();
  AbstractParameter & front();
  const AbstractParameter & back() const;
  const AbstractParameter & front() const;

  void setSerializable(bool serializable);
  bool isSerializable() const;
  bool isReadOnly() const;
  std::shared_ptr<AbstractParameter> newReference() const;

  void setParent(Group & parent);

  operator bool() const;

  ofEvent<AbstractParameter> & parameterChangedE();

  std::vector<std::shared_ptr<AbstractParameter> >::iterator begin();
  std::vector<std::shared_ptr<AbstractParameter> >::iterator end();
  std::vector<std::shared_ptr<AbstractParameter> >::const_iterator begin() const;
  std::vector<std::shared_ptr<AbstractParameter> >::const_iterator end() const;
  std::vector<std::shared_ptr<AbstractParameter> >::reverse_iterator rbegin();
  std::vector<std::shared_ptr<AbstractParameter> >::reverse_iterator rend();
  std::vector<std::shared_ptr<AbstractParameter> >::const_reverse_iterator rbegin() const;
  std::vector<std::shared_ptr<AbstractParameter> >::const_reverse_iterator rend() const;

protected:
  const void* getInternalObject() const;

private:
  class Value{
  public:
    Value()
    :serializable(true){}

    void notifyParameterChanged(AbstractParameter & param);

    std::map<std::string,std::size_t> parametersIndex;
    std::vector<std::shared_ptr<AbstractParameter> > parameters;
    std::string name;
    
    bool serializable;
    std::vector<std::weak_ptr<Value>> parents;
    ofEvent<AbstractParameter> parameterChangedE;
  };
  std::shared_ptr<Value> obj;
  Group(std::shared_ptr<Value> obj)
  :obj(obj){}

  template<typename T>
  friend class Parameter;

  const Group getFirstParent() const;
};

template<typename ParameterType>
const Parameter<ParameterType> & Group::get(const std::string& name) const{
  return static_cast<const Parameter<ParameterType>& >(get(name));
}

template<typename ParameterType>
const Parameter<ParameterType> & Group::get(std::size_t pos) const{
  return static_cast<const Parameter<ParameterType>& >(get(pos));
}

template<typename ParameterType>
Parameter<ParameterType> & Group::get(const std::string& name){
  return static_cast<Parameter<ParameterType>& >(get(name));
}

template<typename ParameterType>
Parameter<ParameterType> & Group::get(std::size_t pos){
  return static_cast<Parameter<ParameterType>& >(get(pos));
}

#pragma mark - Parameter

template<typename ParameterType>
class Parameter: public AbstractParameter{
public:
  Parameter();
  Parameter(const Parameter<ParameterType> & v);
  Parameter(const ParameterType & v, const std::string & unit = "");
  Parameter(const std::string& name, const ParameterType & v, const std::string & unit = "");
  Parameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit = "");

  const ParameterType & get() const;
  const ParameterType * operator->() const;
  operator const ParameterType & () const;

  void setName(const std::string & name);
  std::string getName() const;
  
  void setUnit(const std::string & unit);
  std::string getUnit() const;
  inline bool hasUnit() const { return obj->unit != ""; };
  
  ParameterType getMin() const;
  ParameterType getMax() const;
  ParameterType getInit() const;
  void reInit();

  std::string toString() const;
  void fromString(const std::string & name);

  template<class ListenerClass, typename ListenerMethod>
  void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    ofAddListener(obj->changedE,listener,method,prio);
  }

  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    ofRemoveListener(obj->changedE,listener,method,prio);
  }

  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return obj->changedE.newListener(args...);
  }

  void enableEvents();
  void disableEvents();
  bool isSerializable() const;
  bool isReadOnly() const;
  std::string valueType() const;

  void makeReferenceTo(Parameter<ParameterType> & mom);

  Parameter<ParameterType> & operator=(const Parameter<ParameterType> & v);
  const ParameterType & operator=(const ParameterType & v);

  ParameterType operator++(int v);
  Parameter<ParameterType> & operator++();

  ParameterType operator--(int v);
  Parameter<ParameterType> & operator--();

  template<typename OtherType>
  Parameter<ParameterType> & operator+=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator-=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator*=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator/=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator%=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator&=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator|=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator^=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator<<=(const OtherType & v);
  template<typename OtherType>
  Parameter<ParameterType> & operator>>=(const OtherType & v);


  Parameter<ParameterType> & set(const ParameterType & v, const std::string & unit = "");
  Parameter<ParameterType> & set(const std::string& name, const ParameterType & v, const std::string & unit = "");
  Parameter<ParameterType> & set(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit = "");

  Parameter<ParameterType> & setWithoutEventNotifications(const ParameterType & v);

  void setMin(const ParameterType & min);
    void setMax(const ParameterType & max);
    void setInit(const ParameterType & init);

  void setSerializable(bool serializable);
  std::shared_ptr<AbstractParameter> newReference() const;

  void setParent(Group & _parent);

  const Group getFirstParent() const{
    obj->parents.erase(std::remove_if(obj->parents.begin(),obj->parents.end(),
               [](std::weak_ptr<Group::Value> p){return p.lock()==nullptr;}),
            obj->parents.end());
    if(!obj->parents.empty()){
      return obj->parents.front().lock();
    }else{
      return std::shared_ptr<Group::Value>(nullptr);
    }
  }

  size_t getNumListeners() const;
  const void* getInternalObject() const;

protected:

private:
  class Value{
  public:
    Value()
    :init(of::priv::TypeInfo<ParameterType>::min())
    ,min(of::priv::TypeInfo<ParameterType>::min())
    ,max(of::priv::TypeInfo<ParameterType>::max())
    ,bInNotify(false)
    ,serializable(true){}

    Value(ParameterType v)
    :init(v)
    ,value(v)
    ,min(of::priv::TypeInfo<ParameterType>::min())
    ,max(of::priv::TypeInfo<ParameterType>::max())
    ,bInNotify(false)
    ,serializable(true){}

    Value(std::string name, ParameterType v)
    :name(name)
        ,init(v)
    ,value(v)
    ,min(of::priv::TypeInfo<ParameterType>::min())
    ,max(of::priv::TypeInfo<ParameterType>::max())
    ,bInNotify(false)
    ,serializable(true){}

    Value(std::string name, ParameterType v, ParameterType min, ParameterType max)
    :name(name)
        ,init(v)
    ,value(v)
    ,min(min)
    ,max(max)
    ,bInNotify(false)
    ,serializable(true){}

    std::string name;
    std::string unit { "" };
    ParameterType init, value, min, max;
    ofEvent<ParameterType> changedE;
    bool bInNotify;
    bool serializable;
    std::vector<std::weak_ptr<Group::Value>> parents;
  };

  std::shared_ptr<Value> obj;
  std::function<void(const ParameterType & v)> setMethod;

  void eventsSetValue(const ParameterType & v);
  void noEventsSetValue(const ParameterType & v);
};


template<typename ParameterType>
Parameter<ParameterType>::Parameter()
:obj(std::make_shared<Value>())
,setMethod(std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1)){}

template<typename ParameterType>
Parameter<ParameterType>::Parameter(const Parameter<ParameterType> & v)
:obj(v.obj)
,setMethod(std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1)) {}

template<typename ParameterType>
Parameter<ParameterType>::Parameter(const ParameterType & v, const std::string & unit)
:obj(std::make_shared<Value>(v))
,setMethod(std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1)) {
  setUnit(unit);
}

template<typename ParameterType>
Parameter<ParameterType>::Parameter(const std::string& name, const ParameterType & v, const std::string & unit)
:obj(std::make_shared<Value>(name, v))
,setMethod(std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1)){
  setUnit(unit);
}

template<typename ParameterType>
Parameter<ParameterType>::Parameter(const std::string& name, const ParameterType & v, const ParameterType & min, const ParameterType & max, const std::string & unit)
:obj(std::make_shared<Value>(name, v, min, max))
,setMethod(std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1)){
  setUnit(unit);
}


template<typename ParameterType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator=(const Parameter<ParameterType> & v){
  set(v);
  return *this;
}

template<typename ParameterType>
inline const ParameterType & Parameter<ParameterType>::operator=(const ParameterType & v){
  set(v);
  return obj->value;
}

template<typename ParameterType>
inline Parameter<ParameterType> & Parameter<ParameterType>::set(const ParameterType & v, const std::string & unit){
  setMethod(v);
  setUnit(unit);
  return *this;
}

template<typename ParameterType>
Parameter<ParameterType> & Parameter<ParameterType>::set(const std::string& name, const ParameterType & value, const ParameterType & min, const ParameterType & max, const std::string & unit){
  setName(name);
  set(value);
  setMin(min);
  setMax(max);
    setInit(value);
  setUnit(unit);
  return *this;
}

template<typename ParameterType>
Parameter<ParameterType> & Parameter<ParameterType>::set(const std::string& name, const ParameterType & value, const std::string & unit){
  setName(name);
  set(value);
  setUnit(unit);
  return *this;
}

template<typename ParameterType>
inline Parameter<ParameterType> & Parameter<ParameterType>::setWithoutEventNotifications(const ParameterType & v){
  noEventsSetValue(v);
  return *this;
}

template<typename ParameterType>
inline const ParameterType & Parameter<ParameterType>::get() const{
  return obj->value;
}
template<typename ParameterType>
inline const ParameterType * Parameter<ParameterType>::operator->() const{
  return &obj->value;
}

template<typename ParameterType>
inline void Parameter<ParameterType>::eventsSetValue(const ParameterType & v){

  // If the object is notifying its parents, just set the value without triggering an event.
  if(obj->bInNotify)
  {
    noEventsSetValue(v);
  }
  else
  {
    // Mark the object as in its notification loop.
    obj->bInNotify = true;

    // Set the value.
    obj->value = v;

    // Notify any local subscribers.
    ofNotifyEvent(obj->changedE,obj->value,this);

    // Notify all parents, if there are any.
    if(!obj->parents.empty())
    {
      // Erase each invalid parent
      obj->parents.erase(std::remove_if(obj->parents.begin(),
                        obj->parents.end(),
                        [](const std::weak_ptr<Group::Value> & p){ return p.expired(); }),
                 obj->parents.end());

      // notify all leftover (valid) parents of this object's changed value.
      // this can't happen in the same iterator as above, because a notified listener
      // might perform similar cleanups that would corrupt our iterator
      // (which appens for example if the listener calls getFirstParent on us)
      for(auto & parent: obj->parents){
        auto p = parent.lock();
        if(p){
          p->notifyParameterChanged(*this);
        }
      }
    }
    obj->bInNotify = false;
  }
}

template<typename ParameterType>
inline void Parameter<ParameterType>::noEventsSetValue(const ParameterType & v){
  obj->value = v;
}


template<typename ParameterType>
void Parameter<ParameterType>::setSerializable(bool serializable){
  obj->serializable = serializable;
}

template<typename ParameterType>
bool Parameter<ParameterType>::isSerializable() const{
  return of::priv::has_stream_operators<ParameterType>::value && obj->serializable;
}

template<typename ParameterType>
bool Parameter<ParameterType>::isReadOnly() const{
  return false;
}

template<typename ParameterType>
std::string Parameter<ParameterType>::valueType() const{
  return typeid(ParameterType).name();
}

template<typename ParameterType>
void Parameter<ParameterType>::setMin(const ParameterType & min){
  obj->min = min;
}

template<typename ParameterType>
ParameterType Parameter<ParameterType>::getMin() const {
  return obj->min;
}

template<typename ParameterType>
void Parameter<ParameterType>::setMax(const ParameterType & max){
    obj->max = max;
}

template<typename ParameterType>
ParameterType Parameter<ParameterType>::getMax() const {
    return obj->max;
}

template<typename ParameterType>
void Parameter<ParameterType>::setInit(const ParameterType & init){
    obj->init = init;
}

template<typename ParameterType>
ParameterType Parameter<ParameterType>::getInit() const {
    return obj->init;
}

template<typename ParameterType>
void Parameter<ParameterType>::reInit() {
    setMethod(obj->init);
}

template<typename ParameterType>
inline Parameter<ParameterType>::operator const ParameterType & () const{
  return obj->value;
}

template<typename ParameterType>
void Parameter<ParameterType>::setName(const std::string & name){
  obj->name = name;
}

template<typename ParameterType>
std::string Parameter<ParameterType>::getName() const{
  return obj->name;
}

template<typename ParameterType>
void Parameter<ParameterType>::setUnit(const std::string & unit){
  obj->unit = unit;
}

template<typename ParameterType>
std::string Parameter<ParameterType>::getUnit() const {
  return obj->unit;
}

template<typename ParameterType>
inline std::string Parameter<ParameterType>::toString() const{
  try{
    return of::priv::toStringImpl(obj->value);
  }catch(...){
    ofLogError("Parameter") << "Trying to serialize non-serializable parameter";
    return "";
  }
}

template<typename ParameterType>
inline void Parameter<ParameterType>::fromString(const std::string & str){
  try{
    set(of::priv::fromStringImpl<ParameterType>(str));
  }catch(...){
    ofLogError("Parameter") << "Trying to de-serialize non-serializable parameter";
  }
}

template<typename ParameterType>
void Parameter<ParameterType>::enableEvents(){
  setMethod = std::bind(&Parameter<ParameterType>::eventsSetValue, this, std::placeholders::_1);
}

template<typename ParameterType>
void Parameter<ParameterType>::disableEvents(){
  setMethod = std::bind(&Parameter<ParameterType>::noEventsSetValue, this, std::placeholders::_1);
}

template<typename ParameterType>
inline ParameterType Parameter<ParameterType>::operator++(int){
  ParameterType r = obj->value;
  obj->value++;
  set(obj->value);
  return r;
}

template<typename ParameterType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator++(){
  ++obj->value;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
inline ParameterType Parameter<ParameterType>::operator--(int){
  ParameterType r = obj->value;
  obj->value--;
  set(obj->value);
  return r;
}

template<typename ParameterType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator--(){
  --obj->value;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator+=(const OtherType & v){
  obj->value+=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator-=(const OtherType & v){
  obj->value-=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator*=(const OtherType & v){
  obj->value*=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator/=(const OtherType & v){
  obj->value/=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator%=(const OtherType & v){
  obj->value%=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator&=(const OtherType & v){
  obj->value&=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
Parameter<ParameterType> & Parameter<ParameterType>::operator|=(const OtherType & v){
  obj->value|=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator^=(const OtherType & v){
  obj->value^=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator<<=(const OtherType & v){
  obj->value<<=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
template<typename OtherType>
inline Parameter<ParameterType> & Parameter<ParameterType>::operator>>=(const OtherType & v){
  obj->value>>=v;
  set(obj->value);
  return *this;
}

template<typename ParameterType>
void Parameter<ParameterType>::makeReferenceTo(Parameter<ParameterType> & mom){
  obj = mom.obj;
}

template<typename ParameterType>
std::shared_ptr<AbstractParameter> Parameter<ParameterType>::newReference() const{
  return std::make_shared<Parameter<ParameterType>>(*this);
}

template<typename ParameterType>
void Parameter<ParameterType>::setParent(Group & parent){
  obj->parents.emplace_back(parent.obj);
}

template<typename ParameterType>
size_t Parameter<ParameterType>::getNumListeners() const{
  return obj->changedE.size();
}

template<typename ParameterType>
const void* Parameter<ParameterType>::getInternalObject() const{
  return obj.get();
}

template<>
class Parameter<void>: public AbstractParameter{
public:
  Parameter();
  Parameter(const std::string& name);

  Parameter<void>& set(const std::string & name);

  void setName(const std::string & name);
  std::string getName() const;

  std::string toString() const;
  void fromString(const std::string & name);

  template<class ListenerClass, typename ListenerMethod>
  void addListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    ofAddListener(obj->changedE,listener,method,prio);
  }

  template<class ListenerClass, typename ListenerMethod>
  void removeListener(ListenerClass * listener, ListenerMethod method, int prio=OF_EVENT_ORDER_AFTER_APP){
    ofRemoveListener(obj->changedE,listener,method,prio);
  }

  template<typename... Args>
  std::unique_ptr<of::priv::AbstractEventToken> newListener(Args...args) {
    return obj->changedE.newListener(args...);
  }

  void trigger();
  void trigger(const void * sender);

  void enableEvents();
  void disableEvents();
  bool isSerializable() const;
  bool isReadOnly() const;
  std::string valueType() const;

  void makeReferenceTo(Parameter<void> & mom);

  void setSerializable(bool serializable);
  std::shared_ptr<AbstractParameter> newReference() const;

  void setParent(Group & _parent);

  const Group getFirstParent() const{
    auto first = std::find_if(obj->parents.begin(),obj->parents.end(),[](std::weak_ptr<Group::Value> p){return p.lock()!=nullptr;});
    if(first!=obj->parents.end()){
      return first->lock();
    }else{
      return std::shared_ptr<Group::Value>(nullptr);
    }
  }
  size_t getNumListeners() const;

  const void* getInternalObject() const{
    return obj.get();
  }
protected:

private:
  class Value{
  public:
    Value()
    :serializable(false){}

    Value(std::string name)
    :name(name)
    ,serializable(false){}

    std::string name;
    ofEvent<void> changedE;
    bool serializable;
    std::vector<std::weak_ptr<Group::Value>> parents;
  };
  std::shared_ptr<Value> obj;
};

};
