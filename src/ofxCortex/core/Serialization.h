#pragma once

#include "ofJson.h"

#include "ofxCortex/types/AllTypes.h"
#include <type_traits>

template <typename T>
using ParamType = typename std::decay<decltype(std::declval<T>().get())>::type;


template <typename, typename = void> struct has_toJSON : std::false_type {};
template <typename T> struct has_toJSON<T, std::void_t<decltype(std::declval<T>().toJSON())>> : std::true_type {};

template <typename, typename = void> struct has_fromJSON : std::false_type {};
template <typename T> struct has_fromJSON<T, std::void_t<decltype(std::declval<T>().fromJSON(std::declval<const ofJson&>()))>> : std::true_type {};

template <typename Func>
void deduceParameter(ofAbstractParameter& param, Func func) {
  if (auto p = dynamic_cast<ofParameter<int>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<float>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<bool>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<std::string>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<glm::vec2>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<glm::vec3>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofFloatColor>*>(&param)) { func(*p); }
  
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::Select<int>>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::BeatDivision>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::Palette>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::Range>*>(&param)) { func(*p); }
//  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::OutputValue>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::types::Plot>*>(&param)) { func(*p); }
  
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::LinearFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::InvertFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::GainFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::PulseWidthFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::ExponentialFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::BiasedGainFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::ParabolaFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::ExpBlendFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::ExpStepFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::SmoothstepFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::SineFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::SignedSineFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::PeriodicSineFunction>*>(&param)) { func(*p); }
  else if (auto p = dynamic_cast<ofParameter<ofxCortex::core::utils::TriangleFunction>*>(&param)) { func(*p); }
}

namespace ofxCortex {

inline void deserialize(const ofJson & json, ofAbstractParameter & parameter){
  if (!parameter.isSerializable()) { return; }
  
  std::string name = parameter.getEscapedName();
  
  if (json.find(name) != json.end()){
    if (parameter.type() == typeid(ofParameterGroup).name()){
      ofParameterGroup & group = static_cast<ofParameterGroup &>(parameter);
      
      for (auto & p: group){
        deserialize(json[name], *p);
      }
    }
    else
    {
      deduceParameter(parameter, [&](auto &param) {
        using ValueType = ParamType<decltype(param)>;
        
        auto & casted = parameter.cast<ValueType>();
        auto ref = casted.get();
        
        if constexpr (has_fromJSON<ValueType>::value) {
          ref.fromJSON(json[name]);
        }
        else if (!json[name].is_object())
        {
          std::stringstream ss;
          ss << json[name].get<std::string>();
          ss >> ref;
        }
        
        casted.set(ref);
      });
    }
  }
}

inline void serialize(ofJson & js, ofAbstractParameter & parameter){
  if (!parameter.isSerializable()) { return; }
  std::string name = parameter.getEscapedName();
  
  if (name == "") { name = "UnknownName"; }
  
  if (parameter.type() == typeid(ofParameterGroup).name()){
    const ofParameterGroup & group = static_cast<const ofParameterGroup &>(parameter);
    auto jsonGroup = js[name];
    for (auto & p : group) { serialize(jsonGroup, *p); }
    js[name] = jsonGroup;
  }
  else
  {
    deduceParameter(parameter, [&](auto & param) {
      using ValueType = ParamType<decltype(param)>;
      
      if constexpr (has_toJSON<ValueType>::value) {
        auto ref = parameter.cast<ValueType>().get();
        js[name] = ref.toJSON();
      }
      else
      {
        js[name] = parameter.toString();
      }
    });
  }
}

}
