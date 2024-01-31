#pragma once

#include "ofThread.h"
#include "ofTimer.h"
#include "ofParameterGroup.h"

namespace ofxCortex { namespace core { namespace utils {

#pragma mark - Music
namespace Music {

static float BPMtoPeriod(float BPM)     { return 60.0 / BPM; }
static float PeriodToBPM(float period)  { return 60.0 / period; }

}

#pragma mark - ADSR
class ADSR : public ofThread {
public:
  enum class State {
    IDLE = 0,
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
  };
  
public:
  ADSR() {
    reset();
    setAttack(0);
    setDecay(0);
    setRelease(0);
    setSustainLevel(1.0);
    setTargetRatioA(0.3);
    setTargetRatioDR(0.0001);
    
    startThread();
    timer.setPeriodicEvent(1000000);
    
    parameters.setName("ADSR");
    parameters.add(attackRateParameter, decayRateParameter, releaseRateParameter, sustainLevelParameter);
    listeners.push(attackRateParameter.newListener([this](int & param){ this->setAttack((double) param); }));
    listeners.push(decayRateParameter.newListener([this](int & param){ this->setDecay((double) param); }));
    listeners.push(releaseRateParameter.newListener([this](int & param){ this->setRelease((double) param); }));
    listeners.push(sustainLevelParameter.newListener([this](float & param){ this->setSustainLevel((double) param); }));
  }
  
  ~ADSR() {
    if (isThreadRunning()){
      waitForThread(true, 5000);
    }
  }
  
  void gate(bool status) {
    if (status) {
      state = State::ATTACK;
    } else if (state != State::IDLE) {
      state = State::RELEASE;
    }
  }
  
  void trigger() { this->gate(true); }
  void release() { this->gate(false); };
  void triggerAndRelease() {
    this->shouldRelease = true;
    this->trigger();
  };
  
  double getOutput() {
    return output;
  }
  inline operator double() const { return output; }
  
  void setName(const std::string &name)
  {
    parameters.setName(name);
  }
  
  State getState() {
    return state;
  }
  
  std::string getStateString() {
    switch (state) {
      case State::IDLE: return "Idle";
      case State::ATTACK: return "Attack";
      case State::DECAY: return "Decay";
      case State::SUSTAIN: return "Sustain";
      case State::RELEASE: return "Release";
    }
  }
  
  void setAttack(double rate) {
    attackRate = rate;
    attackCoef = calcCoef(rate, targetRatioA);
    attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);
  }
  
  void setDecay(double rate) {
    decayRate = rate;
    decayCoef = calcCoef(rate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
  }
  
  void setRelease(double rate) {
    releaseRate = rate;
    releaseCoef = calcCoef(rate, targetRatioDR);
    releaseBase = -targetRatioDR * (1.0 - releaseCoef);
  }
  
  void setSustainLevel(double level) {
    sustainLevel = level;
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
  }
  
  void setTargetRatioA(double targetRatio) {
    if (targetRatio < 0.000000001)
      targetRatio = 0.000000001;  // -180 dB
    targetRatioA = targetRatio;
    attackCoef = calcCoef(attackRate, targetRatioA);
    attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);
  }
  
  void setTargetRatioDR(double targetRatio) {
    if (targetRatio < 0.000000001)
      targetRatio = 0.000000001;  // -180 dB
    targetRatioDR = targetRatio;
    decayCoef = calcCoef(decayRate, targetRatioDR);
    releaseCoef = calcCoef(releaseRate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0 - decayCoef);
    releaseBase = -targetRatioDR * (1.0 - releaseCoef);
  }
  
  void reset() {
    state = State::IDLE;
    output = 0.0;
  }
  
  ofParameterGroup parameters;
  
protected:
  ofTimer timer;
  void threadedFunction() {
    while(isThreadRunning()) {
      timer.waitNext();
      
      lock();
      {
        process();
      }
      unlock();
    }
  }
  
  void process() {
    switch (state) {
      case State::IDLE:
        break;
      case State::ATTACK:
        output = attackBase + output * attackCoef;
        if (output >= 1.0) {
          output = 1.0;
          state = State::DECAY;
        }
        break;
      case State::DECAY:
        output = decayBase + output * decayCoef;
        if (output <= sustainLevel) {
          output = sustainLevel;
          state = State::SUSTAIN;
        }
        break;
      case State::SUSTAIN:
        if (shouldRelease) state = State::RELEASE;
        break;
      case State::RELEASE:
        output = releaseBase + output * releaseCoef;
        shouldRelease = false;
        if (output <= 0.0) {
          output = 0.0;
          state = State::IDLE;
        }
    }
  }
  
  ofEventListeners listeners;
  ofParameter<int> attackRateParameter { "Attack", 0, 0, 4000 };
  ofParameter<int> decayRateParameter { "Decay", 0, 0, 4000 };
  ofParameter<int> releaseRateParameter { "Release", 0, 0, 4000 };
  ofParameter<float> sustainLevelParameter { "Sustain", 1.0f, 0.0f, 1.0f };
  
  State state;
  bool shouldRelease { false };
  
  double output;
  double attackRate;
  double decayRate;
  double releaseRate;
  double sustainLevel;
  
  double attackCoef;
  double decayCoef;
  double releaseCoef;
  
  double targetRatioA;
  double targetRatioDR;
  
  double attackBase;
  double decayBase;
  double releaseBase;
  
  double calcCoef(double rate, double targetRatio) {
    return (rate <= 0) ? 0.0 : exp(-log((1.0 + targetRatio) / targetRatio) / rate);
  }
};


#pragma mark - Beat
class Beat : public ofThread {
public:
  enum BeatType : int {
    BAR = 0,
    HALF = 1,
    QUARTER = 2,
    EIGHT = 3,
    SIXTEENTH = 4,
    THIRTYSECOND = 5
  };
  
  struct BeatData : public ofEventArgs {
    bool isBar { false };
    bool isHalf { false };
    bool isQuarter { false };
    bool is8th { false };
    bool is16th { false };
    bool is32th { false };
    
    bool operator[](int i) const {
      switch (i) {
        case 0: return isBar;
        case 1: return isHalf;
        case 2: return isQuarter;
        case 3: return is8th;
        case 4: return is16th;
        case 5: return is32th;
        default: return isQuarter;
      }
    }
    
    friend std::ostream& operator<<(std::ostream& os, const BeatData& beat)
    {
      os << "1/1 = " << beat.isBar << " 1/2 = " << beat.isHalf << " 1/4 = " << beat.isQuarter << " 1/8 = " << beat.is8th << " 1/16 = " << beat.is16th << " 1/32 = " << beat.is32th;
      return os;
    }
  };
  
public:
  Beat() {
    this->setBPM(120);
    startThread();
    
    parameters.setName("Beat");
    parameters.add(BPM.set("BPM", 120, 60, 280));
    BPM.addListener(this, &Beat::setBPMInternal);
  }
  
  ~Beat() {
    BPM.removeListener(this, &Beat::setBPMInternal);
    
    if (isThreadRunning()){
      waitForThread(true, 5000);
    }
  }
  
  inline void reset()
  {
    timer.reset();
    beat = 0;
  }
  
  inline void setBPM(double bpm)
  {
    this->BPM.setWithoutEventNotifications(bpm);
    timer.setPeriodicEvent(BPMtoNano(this->BPM * 32 / 4.0));
    reset();
  }
  
  inline double getBPM() const { return BPM; }
  
  inline BeatData getBeatData()
  {
    BeatData data;
    
    lock();
    {
      data.isBar = beat % 32 == 0;
      data.isHalf = beat % 16 == 0;
      data.isQuarter = beat % 8 == 0;
      data.is8th = beat % 4 == 0;
      data.is16th = beat % 2 == 0;
      data.is32th = beat % 1 == 0;
    }
    unlock();
    
    return data;
  }
  
  inline static int size() { return 5; }
  
  ofEvent<BeatData> onBeatE;
  
  ofParameterGroup parameters;
  
protected:
  ofTimer timer;
  unsigned long beat { 0 };
  
  ofParameter<int> BPM { "BPM", 120, 60, 200 };
  void setBPMInternal(int & param) { this->setBPM(param); }
  
  inline uint64_t BPMtoNano(double bpm)
  {
    double seconds = 60.0 / bpm;
    return seconds * 1000000000;
  }
  
  inline void threadedFunction() {
    while(isThreadRunning()) {
      timer.waitNext();
      
      lock();
      {
        beat = (beat + 1) % 32;
      }
      unlock();
      
      BeatData event;
      event.isBar = beat % 32 == 0;
      event.isHalf = beat % 16 == 0;
      event.isQuarter = beat % 8 == 0;
      event.is8th = beat % 4 == 0;
      event.is16th = beat % 2 == 0;
      event.is32th = beat % 1 == 0;
      
      onBeatE.notify(this, event);
    }
  }
  
};

inline std::ostream& operator<<(std::ostream& os, const Beat::BeatType& type)
{
  switch (type) {
    case Beat::BAR: os << "1/1 Bar"; break;
    case Beat::HALF: os << "1/2 Bar"; break;
    case Beat::QUARTER: os << "1/4 Bar"; break;
    case Beat::EIGHT: os << "1/8 Bar"; break;
    case Beat::SIXTEENTH: os << "1/16 Bar"; break;
    case Beat::THIRTYSECOND: os << "1/32 Bar"; break;
    default: os << "1/4 Bar"; break;
  }
  return os;
}

#pragma mark - BeatMatcher
class BeatMatcher {
public:
  BeatMatcher() {
    parameters.setName("Beat Matcher");
    parameters.add(triggerButton);
    
    triggerButton.addListener(this, &BeatMatcher::trigger);
  }
  
  ~BeatMatcher() {
    triggerButton.removeListener(this, &BeatMatcher::trigger);
  }
  
  void trigger()
  {
    float time = ofGetElapsedTimef();
    float diff = time - lastTriggerTime;
    
    if (diff > 10.0f) differences.clear();
    
    differences.push_back(diff);
    
    if (differences.size() > 12)
    {
      differences.pop_front();
      
      float BPM = 60.0 / Array::average(differences);
      onMatch.notify(this, BPM);
    }
    
    lastTriggerTime = time;
  }
  
  ofEvent<float> onMatch;
  
  ofParameterGroup parameters;
  
protected:
  ofParameter<void> triggerButton { "Trigger" };
  float lastTriggerTime { 0.0f };
  std::deque<float> differences;
};

}}}
