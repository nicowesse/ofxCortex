#pragma once

namespace ofxCortex { namespace core { namespace generators {

class Waveform {
public:
//  static double sine(WaveformSettings settings)
//  {
//    return ofMap(sin(settings.cycle()), -1.0, 1.0, (settings.unsign) ? 0.0 : -settings.amplitude, settings.amplitude);
//  }
  
  static double sine(double x, double period, double amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap(sin(cycle(x, period), shift), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
//  static double sawtooth(WaveformSettings settings)
//  {
//    auto cot = [](float x) { return tan(HALF_PI - x); };
//
//    float trig = (2.0 / PI) * atan(cot(settings.cycle())) * -1.0;
//
//    return ofMap(trig, -1.0, 1.0, (settings.unsign) ? 0.0 : -settings.amplitude, settings.amplitude);
//  }
  
  static double sawtooth(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    auto cot = [](float x) { return tan(HALF_PI - x); };
    
    double trig = (2.0 / PI) * atan(cot(cycle(x, period, shift))) * -1.0;
    
    return ofMap(trig, -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
//  static double triangle(WaveformSettings settings)
//  {
//    return ofMap((2.0 / PI) * asin(sine(settings.x, settings.frequency, 1.0, settings.shift)), -1.0, 1.0, (settings.unsign) ? 0.0 : -settings.amplitude, settings.amplitude);
//  }
  
  static double triangle(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap((2.0 / PI) * asin(sine(x, period, 1.0, shift)), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
//  static double square(WaveformSettings settings)
//  {
//    return ofMap(ofSign(sine(settings.x, settings.frequency, 1.0, settings.shift)), -1.0, 1.0, (settings.unsign) ? 0.0 : -settings.amplitude, settings.amplitude);
//  }
  
  static double square(double x, double period, float amplitude = 1.0, double shift = 0.0, bool unsign = false)
  {
    return ofMap(ofSign(sine(x, period, 1.0, shift)), -1.0, 1.0, (unsign) ? 0.0 : -amplitude, amplitude);
  }
  
  
protected:
  double cycle(double x, double period, double shift)
  {
    return TWO_PI * (x - fmod(shift, period)) * p2f(period);
  }
  
  double f2p(double freq) { return 1.0 / freq; }
  double p2f(double period) { return 1.0 / period; }
  
  double period(double freq) { return 1.0 / freq; }
  double frequency(double period) { return 1.0 / period; }
  
};

}}}
