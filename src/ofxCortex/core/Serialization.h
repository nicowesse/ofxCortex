#pragma once

#include "ofJson.h"
#include "ofxCortex/types/Parameter.h"

namespace ofCortex::core {
inline void serialize(ofJson & js, const ofxCortex::types::AbstractParameter & parameter){
  if(!parameter.isSerializable()){
    return;
  }
  std::string name = parameter.getEscapedName();
  if(name == ""){
    name = "UnknownName";
  }
  if(parameter.type() == typeid(ofxCortex::types::Group).name()){
    const Group & group = static_cast <const ofxCortex::types::Group &>(parameter);
    auto jsonGroup = js[name];
    for(auto & p: group){
      serialize(jsonGroup, *p);
    }
    js[name] = jsonGroup;
  }else{
    std::string value = parameter.toString();
    js[name] = value;
  }
}

inline void deserialize(const ofJson & json, ofxCortex::types::AbstractParameter & parameter){
  if(!parameter.isSerializable()){
    return;
  }
  std::string name = parameter.getEscapedName();
  if(json.find(name) != json.end()){
    if(parameter.type() == typeid(ofxCortex::types::Group).name()){
      Group & group = static_cast <ofxCortex::types::Group &>(parameter);
      for(auto & p: group){
        deserialize(json[name], *p);
      }
    }else{
      if(parameter.type() == typeid(ofxCortex::types::Parameter <int> ).name() && json[name].is_number_integer()){
        parameter.cast <int>() = json[name].get<int>();
      }else if(parameter.type() == typeid(ofxCortex::types::Parameter <float> ).name() && json[name].is_number_float()){
        parameter.cast <float>() = json[name].get<float>();
      }else if(parameter.type() == typeid(ofxCortex::types::Parameter <bool> ).name() && json[name].is_boolean()){
        parameter.cast <bool>() = json[name].get<bool>();
      }else if(parameter.type() == typeid(ofxCortex::types::Parameter <int64_t> ).name() && json[name].is_number_integer()){
        parameter.cast <int64_t>() = json[name].get<int64_t>();
      }else if(parameter.type() == typeid(ofxCortex::types::Parameter <std::string> ).name()){
        parameter.cast <std::string>() = json[name].get<std::string>();
      }else{
        parameter.fromString(json[name]);
      }
    }
  }
}

}
