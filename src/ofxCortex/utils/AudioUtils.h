#pragma once

#include "ofThread.h"
#include "ofTimer.h"
#include "ofParameterGroup.h"
#include "ofxCortex/utils/Helpers.h"
#include "ofxCortex/types/BeatDivision.h"

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
  ADSR(const std::string & name = "ADSR", double attack = 0.0, double decay = 0.0, double sustain = 1.0, double release = 1000.0)
  {
    reset();
    setAttack(attack);
    setDecay(decay);
    setSustainLevel(sustain);
    setRelease(release);
    
    setTargetRatioA(0.3);
    setTargetRatioDR(0.0001);
    
    startThread();
    timer.setPeriodicEvent(1000000);
    
    parameters.setName(name);
    parameters.add(triggerParameter, attackRateParameter, decayRateParameter, releaseRateParameter, sustainLevelParameter, outputParameter);
    
    onTrigger = triggerParameter.newListener([this](){
      this->triggerAndRelease();
    });
  }
  
  ADSR(const ADSR& other) 
  {
    state = other.state;
    shouldRelease = other.shouldRelease;
    output = other.output;
    targetRatioA = other.targetRatioA;
    targetRatioDR = other.targetRatioDR;
    shouldRelease = other.shouldRelease;
    
    parameters = other.parameters;
    triggerParameter = other.triggerParameter;
    attackRateParameter = other.attackRateParameter;
    decayRateParameter = other.decayRateParameter;
    releaseRateParameter = other.releaseRateParameter;
    sustainLevelParameter = other.sustainLevelParameter;
    outputParameter = other.outputParameter;
    
    onTrigger = triggerParameter.newListener([this](){
      this->triggerAndRelease();
    });
  }
  
  ADSR& operator=(const ADSR& other) 
  {
    if (this != &other)
    {
      state = other.state;
      shouldRelease = other.shouldRelease;
      output = other.output;
      targetRatioA = other.targetRatioA;
      targetRatioDR = other.targetRatioDR;
      shouldRelease = other.shouldRelease;
      
      parameters = other.parameters;
      triggerParameter = other.triggerParameter;
      attackRateParameter = other.attackRateParameter;
      decayRateParameter = other.decayRateParameter;
      releaseRateParameter = other.releaseRateParameter;
      sustainLevelParameter = other.sustainLevelParameter;
      outputParameter = other.outputParameter;
      
      onTrigger = triggerParameter.newListener([this](){
        this->triggerAndRelease();
      });
    }
    
    return *this;
  }
  
  ~ADSR() {
    if (isThreadRunning()) { waitForThread(true, 5000); }
  }
  
  
  void gate(bool status) {
    if (status) { state = State::ATTACK; }
    else if (state != State::IDLE) { state = State::RELEASE; }
  }
  
  void trigger() { this->gate(true); }
  void release() { this->gate(false); };
  void triggerAndRelease() {
    this->shouldRelease = true;
    this->trigger();
  };
  
  double getOutput() { return output; }
  operator float()  const { return output; }
  operator double() const { return output; }
  
  void setName(const std::string &name) { parameters.setName(name); }
  
  State getState() { return state; }
  
  std::string getStateString() {
    switch (state) {
      case State::IDLE: return "Idle";
      case State::ATTACK: return "Attack";
      case State::DECAY: return "Decay";
      case State::SUSTAIN: return "Sustain";
      case State::RELEASE: return "Release";
    }
  }
  
  double getAttackRate() const { return (double) attackRateParameter.get(); }
  void setAttack(double rate) { attackRateParameter = (int) rate; }
  
  double getDecayRate() const { return (double) decayRateParameter.get(); }
  void setDecay(double rate) { decayRateParameter = (int) rate; }
  
  double getReleaseRate() const { return (double) releaseRateParameter.get(); }
  void setRelease(double rate) { releaseRateParameter = (int) rate; }
  
  double getSustainLevel() const { return (double) sustainLevelParameter.get(); }
  void setSustainLevel(double level) { sustainLevelParameter = (float) level; }
  
  void setTargetRatioA(double targetRatio) { targetRatioA = std::max(0.000000001, targetRatio); }
  
  void setTargetRatioDR(double targetRatio) { targetRatioDR = std::max(0.000000001, targetRatio); }
  
  void reset() {
    state = State::IDLE;
    output = 0.0;
  }
  
  ofParameterGroup parameters;
  operator ofParameterGroup&() { return parameters; }
  
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
      case State::IDLE: break;
      case State::ATTACK:
      {
        double attackCoef = calcCoef(getAttackRate(), targetRatioA);
        double attackBase = (1.0 + targetRatioA) * (1.0 - attackCoef);
        
        output = attackBase + output * attackCoef;
        if (output >= 1.0) {
          output = 1.0;
          state = State::DECAY;
        }
        break;
      }
      case State::DECAY:
      {
        double decayCoef = calcCoef(getDecayRate(), targetRatioDR);
        double decayBase = (getSustainLevel() - targetRatioDR) * (1.0 - decayCoef);
        
        output = decayBase + output * decayCoef;
        if (output <= getSustainLevel()) {
          output = getSustainLevel();
          state = State::SUSTAIN;
        }
        break;
      }
      case State::SUSTAIN:
      {
        if (shouldRelease) state = State::RELEASE;
        break;
      }
      case State::RELEASE:
      {
        double releaseCoef = calcCoef(getReleaseRate(), targetRatioDR);
        double releaseBase = -targetRatioDR * (1.0 - releaseCoef);
        
        output = releaseBase + output * releaseCoef;
        shouldRelease = false;
        if (output <= 0.0) {
          output = 0.0;
          state = State::IDLE;
        }
        break;
      }
    }
    
    outputParameter = ofToString(output, 4);
  }
  
  
  ofParameter<void> triggerParameter { "Trigger" };
  ofEventListener onTrigger;
  
  ofParameter<int> attackRateParameter { "Attack", 0, 0, 4000 };
  ofParameter<int> decayRateParameter { "Decay", 0, 0, 4000 };
  ofParameter<int> releaseRateParameter { "Release", 0, 0, 4000 };
  ofParameter<float> sustainLevelParameter { "Sustain", 1.0f, 0.0f, 1.0f };
  
  ofParameter<std::string> outputParameter { "Output", "0.0" };
  
  State state { State::IDLE };
  bool shouldRelease { false };
  
  double output;
  
  double targetRatioA;
  double targetRatioDR;
  
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
    parameters.add(resetBPM);
    resetBPM.addListener(this, &Beat::reset);
    
    parameters.add(BPM.set("BPM", 120, 60, 280));
    BPM.addListener(this, &Beat::setBPMInternal);
  }
  
  ~Beat() {
    BPM.removeListener(this, &Beat::setBPMInternal);
    resetBPM.removeListener(this, &Beat::reset);
    
    if (isThreadRunning()){
      waitForThread(true, 5000);
    }
  }
  
  inline void reset()
  {
    timer.reset();
    beat = 0;
  }
  
  inline void setBPM(float bpm)
  {
    this->BPM.setWithoutEventNotifications(bpm);
    timer.setPeriodicEvent(BPMtoNano(this->BPM * 32 / 4.0));
    reset();
  }
  
  inline double getBPM() const { return BPM; }
  
  inline BeatData getBeatData() const
  {
    BeatData data;
    
    data.isBar = beat % 32 == 0;
    data.isHalf = beat % 16 == 0;
    data.isQuarter = beat % 8 == 0;
    data.is8th = beat % 4 == 0;
    data.is16th = beat % 2 == 0;
    data.is32th = beat % 1 == 0;
    
    return data;
  }
  
  float getBeatLength(int division) const
  {
    return ((60.0 / getBPM()) * 4.0) / pow(2, division % 5);
  }
  
  inline static int size() { return 5; }
  
  ofEvent<BeatData> onBeatE;
  
  ofParameterGroup parameters;
  
protected:
  ofTimer timer;
  unsigned long beat { 0 };
  
  ofParameter<void> resetBPM { "Reset BPM" };
  
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
      
      BeatData event = getBeatData();
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
