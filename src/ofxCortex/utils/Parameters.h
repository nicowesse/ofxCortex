#pragma once

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
  static ofParameter<T> addParameterWithEvent(const std::string & name, T value, T min, T max, ofParameterGroup & parameters, F&& lambda)
  {
    ofParameter<T> param(name, value, min, max);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda));
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
  static ofParameter<T> addParameterWithEvent(const std::string & name, T value, ofParameterGroup & parameters, F&& lambda)
  {
    ofParameter<T> param(name, value);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda));
    return param;
  }
  
  template<typename T>
  static ofParameter<T> addParameter(const std::string & name, ofParameterGroup & parameters)
  {
    ofParameter<T> param(name);
    parameters.add(param);
    return param;
  }
  
  template<typename T, typename F>
  static ofParameter<T> addParameterWithEvent(const std::string & name, ofParameterGroup & parameters, F&& lambda)
  {
    ofParameter<T> param(name);
    parameters.add(param);
    
    ofxCortex::core::listeners.push(param.newListener(lambda));
    return param;
  }
  
  
private:
  Parameters() = default;
  
};

}}}
