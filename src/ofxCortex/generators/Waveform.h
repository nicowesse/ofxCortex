#pragma once

#include "ofxCortex/utils/ParameterUtils.h"

namespace ofxCortex { namespace core { namespace generators {

class Waveform {
public:
  enum class Type {
    SINE, TRIANGLE, SQUARE, SAWTOOTH
  };
  
  struct Settings {
    Type type { Type::SINE };
    double period { 1.0 };
    double amplitude { 1.0 };
    double shift { 0.0 };
    bool unsign { false };
    
    void BPMtoPeriod(float BPM) { this->period = 60.0 / BPM; }
    
    friend std::ostream& operator<<(std::ostream& os, const Settings& settings)
    {
      os << "Type: " << Waveform::typeToString(settings.type) << " | Period: " << settings.period << " | Amplitude: " << settings.amplitude << " | Shift: " << settings.shift << " | Unsign: " << std::boolalpha << settings.unsign;
      
      return os;
    }
  };
  
public:
  Waveform(const std::string & name = "Waveform") {
    this->parameters = getParameters(name);
    
    settings = getSettings(parameters);
    this->settingsChanged = parameters.parameterChangedE().newListener([&, this](const ofAbstractParameter & param) {
      settings = getSettings(parameters);
    });
  };
  
  operator ofParameterGroup&() { return this->parameters; }
  operator const Settings&() { return this->settings; }
  
  double wave(double x)
  {
    return wave(x, this->settings);
  }
  
  static double wave(double x, Waveform::Type type, double period, double amplitude = 1.0, double shift = 1.0, bool unsign = false)
  {
    switch (type) {
      case Type::SINE: return sine(x, period, amplitude, shift, unsign); break;
      case Type::TRIANGLE: return triangle(x, period, amplitude, shift, unsign); break;
      case Type::SQUARE: return square(x, period, amplitude, shift, unsign); break;
      case Type::SAWTOOTH: return sawtooth(x, period, amplitude, shift, unsign); break;
    }
  }
  
  static double wave(double x, const Waveform::Settings & settings)
  {
    return wave(x, settings.type, settings.period, settings.amplitude, settings.shift, settings.unsign);
  }
  
  
  // Sine
  double sine(double x)
  {
    return Waveform::sine(x, this->settings);
  }
  
  static double sine(double x, double period, double amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap(sin(cycle(x, period, shift)), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  static double sine(double x, const Waveform::Settings & settings)
  {
    return sine(x, settings.period, settings.amplitude, settings.shift, settings.unsign);
  }
  
  // Triangle
  double triangle(double x) { return triangle(x, this->settings); }
  
  static double triangle(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap((2.0 / PI) * asin(sine(x, period, 1.0, shift)), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
  static double triangle(double x, const Waveform::Settings & settings)
  {
    return triangle(x, settings.period, settings.amplitude, settings.shift, settings.unsign);
  }
  
  // Sawtooth
  double sawtooth(double x) { return sawtooth(x, this->settings); }
  
  static double sawtooth(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    auto cot = [](float x) { return tan(HALF_PI - x); };
    
    double trig = (2.0 / PI) * atan(cot(cycle(x, period, shift)));
    
    return ofMap(trig, -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
  static double sawtooth(double x, const Waveform::Settings & settings)
  {
    return sawtooth(x, settings.period, settings.amplitude, settings.shift, settings.unsign);
  }
  
  // Square
  double square(double x) { return square(x, this->settings); }
  
  static double square(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap(ofSign(sine(x, period, 1.0, shift)), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
  static double square(double x, const Waveform::Settings & settings)
  {
    return square(x, settings.period, settings.amplitude, settings.shift, settings.unsign);
  }
  
  static double period(double freq) { return 1.0 / freq; }
  static double frequency(double period) { return 1.0 / period; }
  
  static double f2p(double freq) { return 1.0 / freq; }
  static double p2f(double period) { return 1.0 / period; }
  
  ofParameterGroup parameters;
  
  static void addParameters(ofParameterGroup & parameters)
  {
    ofParameterGroup params = getParameters();
    parameters.add(params);
  }
  
  static ofParameterGroup getParameters(const std::string & name = "Waveform")
  {
    ofParameterGroup waveformParameters { "Waveform" };
    
    utils::Parameters::addParameter<string>("Type", "Sine", waveformParameters);
    utils::Parameters::addParameter<float>("Period", 1.0f, 0.0f, 500.f, waveformParameters);
    utils::Parameters::addParameter<float>("Amplitude", 1.0f, 0.0f, 20.0f, waveformParameters);
    utils::Parameters::addParameter<float>("Shift", 0.0f, -10.f, 10.f, waveformParameters);
    utils::Parameters::addParameter<bool>("Unsigned", true, waveformParameters);
    
    return waveformParameters;
  }
  
  static Settings getSettings(const ofParameterGroup & parameters) {
    Settings s;
    
    s.period = utils::Parameters::getParameter<float>("Period", parameters);
    s.amplitude = utils::Parameters::getParameter<float>("Amplitude", parameters);
    s.shift = utils::Parameters::getParameter<float>("Shift", parameters);
    s.unsign = utils::Parameters::getParameter<bool>("Unsigned", parameters);
    
    return s;
  }
  
  static void draw(const Settings & settings, const ofRectangle & viewport)
  {
    float inset = 16;
    int steps = viewport.width / 2.0f;
    
    ofPushStyle();
    
    ofFill();
    ofSetColor(12);
    ofDrawRectRounded(viewport, 6);

    ofNoFill();
    ofSetColor(255, 32);
    ofDrawRectRounded(viewport, 6);
    
    ofSetColor(64);
    ofDrawLine(viewport.getLeft() + inset, viewport.getCenter().y, viewport.getRight() - inset, viewport.getCenter().y);
    ofDrawLine(viewport.getLeft() + inset, viewport.getCenter().y, viewport.getLeft() + inset, viewport.getCenter().y);
    
    ofFill();
    ofDrawCircle(viewport.getLeft() + inset, viewport.getCenter().y, 2);
    ofDrawCircle(viewport.getRight() - inset, viewport.getCenter().y, 2);
    
    ofNoFill();
    ofSetColor(255);
    ofBeginShape();
    for (int i = 0; i <= steps; i++)
    {
      float t = (float) i / steps;
      float x = ofMap(t, 0, 1, viewport.getLeft() + inset, viewport.getRight() - inset, true);
      float y = ofMap(wave(t, settings), -1, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true);
      
      ofVertex(x, y);
    }
    ofEndShape();
    
    ofFill();
    ofSetColor(128);
    ofDrawCircle(viewport.getLeft() + inset, ofMap(wave(0.0, settings), -1, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
    ofDrawCircle(viewport.getRight() - inset, ofMap(wave(1.0, settings), -1, 1, viewport.getBottom() - inset, viewport.getTop() + inset, true), 2);
    
    ofDrawBitmapString(ofxCortex::core::utils::String::toTitleCase(typeToString(settings.type)), viewport.getLeft() + inset, viewport.getTop() + inset + 11 + 4);
    
    ofSetColor(255, 64);
    ofDrawBitmapString("0", viewport.getLeft() + inset, viewport.getCenter().y - 4);
    ofDrawBitmapString("1", viewport.getRight() - inset - 8, viewport.getCenter().y - 4);
    
    ofPopStyle();
  }
  
protected:
  static double cycle(double x, double period, double shift)
  {
    return TWO_PI * (x - fmod(shift, period)) * p2f(period);
  }
  
  static std::string typeToString(const Type & type) {
    if (type == Type::SINE) { return "sine"; }
    if (type == Type::TRIANGLE) { return "triangle"; }
    if (type == Type::SQUARE) { return "square"; }
    if (type == Type::SAWTOOTH) { return "sawtooth"; }
    
    return "sine";
  }
  
  static Type stringToType(const std::string & type) {
    if (type == "sine") { return Type::SINE; }
    if (type == "triangle") { return Type::TRIANGLE; }
    if (type == "square") { return Type::SQUARE; }
    if (type == "sawtooth") { return Type::SAWTOOTH; }
    
    return Type::SINE;
  }
  
  ofEventListener settingsChanged;
  Settings settings;
  
};

}}}
