#pragma once

#include <chrono>
using namespace std::chrono;

#include "ofxCortex/types/Spacer.h"

namespace ofxCortex { namespace core {

static ofEventListeners listeners;

namespace utils {

class Parameters {
  
public:
  template<typename T>
  static ofParameter<T> addParameter(const std::string & name, T value, T min, T max, ofParameterGroup & parameters)
  {
    ofParameter<T> param(name, value, min, max);
    parameters.add(param);
    return param;
  }
  
  template<typename T, typename F>
  static ofParameter<T> addParameterWithEvent(const std::string & name, T value, T min, T max, ofParameterGroup & parameters, F&& lambda, int priority = OF_EVENT_ORDER_AFTER_APP)
  {
    ofParameter<T> param(name, value, min, max);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda, priority));
    return param;
  }
  
  template<typename T>

  static ofParameter<T> addParameter(const std::string & name, T value, ofParameterGroup & parameters)
  {
    ofParameter<T> param(name, value);
    parameters.add(param);
    return param;
  }
  
  template<typename T, typename F>
  static ofParameter<T> addParameterWithEvent(const std::string & name, T value, ofParameterGroup & parameters, F&& lambda = nullptr, int priority = OF_EVENT_ORDER_AFTER_APP)
  {
    ofParameter<T> param(name, value);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda, priority));
    return param;
  }
  
  template<typename T>
  static ofParameter<T> addParameter(const std::string & name, ofParameterGroup & parameters)
  {
    ofParameter<T> param(name, T());
    parameters.add(param);
    return param;
  }
  
  template<typename T, typename F>
  static ofParameter<T> addParameterWithEvent(const std::string & name, ofParameterGroup & parameters, F&& lambda, int priority = OF_EVENT_ORDER_AFTER_APP)
  {
    ofParameter<T> param(name);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda, priority));
    return param;
  }
  
//  template<typename T>
//  static T getParameter(const std::string & name, const ofParameterGroup & parameters, T defaultValue = T())
//  {
//    if (!parameters.contains(name)) return defaultValue;
//    else return parameters.get<T>(name).get();
//  }
  
  template<typename F>
  static ofParameter<void> addTrigger(const std::string & name, ofParameterGroup & parameters, F&& lambda, int priority = OF_EVENT_ORDER_AFTER_APP)
  {
    ofParameter<void> param(name);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda, priority));
    return param;
  }
  
  template<typename T>
  static T getParameter(const std::string & name, const ofParameterGroup & parameters, T defaultValue = T())
  {
    if (parameters.size() == 0) return defaultValue;
    
    auto searchPath = ofSplitString(name, "::", false, true);
    
    if (!parameters.contains(searchPath[0])) {
      ofLogWarning("getParameter<T>") << "⚠️ '" << searchPath[0] << "' not found in '" << parameters.getName() << "'. Returning default value = '" << defaultValue << "'";
      return defaultValue;
    }
    
    const ofAbstractParameter & foundParameter = parameters.get(searchPath[0]);
    
    if (foundParameter.type() == typeid(ofParameterGroup).name()) return getParameter<T>(ofJoinString(std::vector<std::string>(searchPath.begin() + 1, searchPath.end()), "::"), foundParameter.castGroup(), defaultValue);
    else {
      if (foundParameter.valueType() != typeid(T).name()) ofLogWarning("getParameter<T>") << "⚠️ Wrong type for '" << foundParameter.getName() << "' (actual: " << foundParameter.valueType() << " vs. input: " << typeid(T).name() << "). I'll try to cast it, but might fuck up some shit.";

      return foundParameter.cast<T>();
    }
  }
  
  static void addSpacer(ofParameterGroup & parameters, int spaces = 1)
  {
    static int spacerCount = 0;
    addParameter<ofxCortex::core::types::Spacer>("Spacer " + ofToString(spacerCount++), ofxCortex::core::types::Spacer(spaces), parameters);
  }
  
  static void debugStructure(const ofParameterGroup & group, std::ostream &out, int level = 0) {
    std::stringstream levelStr;
    levelStr << "┣";
    for (int i = 0; i < level; i++) levelStr << "━";
    
    
    out << levelStr.str() << "┫" << group.getName() << "\n";
    for (const auto & param : group)
    {
      if (param->type() == typeid(ofParameterGroup).name()) {
        debugStructure(param->castGroup(), out, level + 1);
      }
      else
      {
        out << levelStr.str() << "━ " << param->getName() << "\n";
      }
    }
  };
  
  
private:
  Parameters() = default;
  
};

}}}
