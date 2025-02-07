#pragma once

#include "ofxCortex/core/Events.h"
#include "ofxCortex/utils/ContainerUtils.h"
#include "ofxCortex/types/Spacer.h"

namespace ofxCortex { namespace core { namespace utils {

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
  
  static const ofAbstractParameter & getParameter(const std::string & name, const ofParameterGroup & parameters)
  {
    if (parameters.size() == 0) return parameters;
    
    auto searchPath = ofSplitString(name, "::", false, true);
    
    if (!parameters.contains(searchPath[0])) 
    {
        ofLogWarning("getParameter") << "⚠️ '" << searchPath[0] << "' not found in '" << parameters.getName() << "'";
        return parameters;
    }
    
    const ofAbstractParameter & foundParameter = parameters.get(searchPath[0]);
    
    if (foundParameter.type() == typeid(ofParameterGroup).name()) return getParameter(ofJoinString(std::vector<std::string>(searchPath.begin() + 1, searchPath.end()), "::"), foundParameter.castGroup());
    else {
      return foundParameter;
    }
  }
  
  static void linkParameters(ofParameterGroup & primary, ofParameterGroup & subordinate)
  {
    
    for (std::shared_ptr<ofAbstractParameter> & parameter : subordinate)
    {
      std::string subName = parameter->getName();
      std::string paramType = parameter->type();
      std::string type = parameter->valueType();
      
      if (!primary.contains(subName)) continue;
      
      ofAbstractParameter & primaryParam = primary.get(subName);
      if (primaryParam.valueType() != parameter->valueType()) continue;
      
      ofLogVerbose("Parameters::linkParameters()") << serializeName(*parameter, false) << " (" << type << ")  =>  " << serializeName(primaryParam, true) << " (" << primaryParam.valueType() << ")";
      
      if (paramType == typeid(ofParameterGroup).name()) { linkParameters(primaryParam.castGroup(), parameter->castGroup()); }
      else if (type == typeid(float).name()) parameter->cast<float>().makeReferenceTo(primaryParam.cast<float>());
      else if (type == typeid(int).name()) parameter->cast<int>().makeReferenceTo(primaryParam.cast<int>());
      else if (type == typeid(bool).name()) parameter->cast<bool>().makeReferenceTo(primaryParam.cast<bool>());
      else if (type == typeid(std::string).name()) parameter->cast<std::string>().makeReferenceTo(primaryParam.cast<std::string>());
      else if (type == typeid(glm::vec2).name()) parameter->cast<glm::vec2>().makeReferenceTo(primaryParam.cast<glm::vec2>());
      else if (type == typeid(glm::vec3).name()) parameter->cast<glm::vec3>().makeReferenceTo(primaryParam.cast<glm::vec3>());
    }
  }
  
  static std::string serializeName(const ofAbstractParameter & parameter, bool skipRoot = true)
  {
    std::vector<std::string> hierarchy = ofxCortex::core::utils::Array::transform<std::string>(parameter.getGroupHierarchyNames(), Parameters::unescape);
    
    return ofJoinString(std::vector<std::string>(hierarchy.begin() + skipRoot, hierarchy.end()), "::");
  }
  
  static std::string getHashedParameterName(const ofAbstractParameter & param)
  {
    static std::hash<std::string> hasher;
    return std::to_string(hasher(serializeName(param, false)));
  }
  
  static std::string hash(const ofAbstractParameter & param) { return getHashedParameterName(param); }
  
  static int getLevel(ofAbstractParameter & param)
  {
    return param.getGroupHierarchyNames().size() - 1;
  }
  
  static std::string unescape(const std::string & _str)
  {
    std::string str(_str);

    ofStringReplace(str, "_", " ");

    return str;
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
    
    
    out << levelStr.str() << group.getName() << " (group)\n";
    for (const auto & param : group)
    {
      if (param->type() == typeid(ofParameterGroup).name()) {
        debugStructure(param->castGroup(), out, level + 1);
      }
      else
      {
        out << levelStr.str() << "━ " << param->getName() << " (" << param->valueType() << ")\n";
      }
    }
  };
  
  
private:
  Parameters() = default;
  
};

}}}
