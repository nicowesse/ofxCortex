#pragma once

#include "ofThread.h"
#include "ofThreadChannel.h"
#include "ofTypes.h"
#include "ofTimer.h"
#include "ofParameterGroup.h"
#include "ofxCortex/utils/Helpers.h"
#include "ofxCortex/utils/ContainerUtils.h"
#include "ofxCortex/types/BeatDivision.h"
#include "ofxCortex/types/Select.h"

namespace ofxCortex { namespace core { namespace utils {

#pragma mark - Music
namespace Music {

inline static float BPMtoPeriod(float BPM)     { return 60.0 / BPM; }
inline static float PeriodToBPM(float period)  { return 60.0 / period; }

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
    
    parameters.setName(name);
    parameters.add(triggerParameter, attackRateParameter, decayRateParameter, releaseRateParameter, sustainLevelParameter);
    
    onTrigger = triggerParameter.newListener([this](){
      this->triggerAndRelease();
    });
    
    timer.setPeriodicEvent(1000000);
    startThread();
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
    waitForThread(true);
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
  operator float()  const {
    ofScopedLock();
    return output;
  }
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
  
  void setAttack(double rate) { attackRateParameter = (int) rate; }
  double getAttackRate() const { return (double) attackRateParameter.get(); }
  
  void setDecay(double rate) { decayRateParameter = (int) rate; }
  double getDecayRate() const { return (double) decayRateParameter.get(); }
  
  void setRelease(double rate) { releaseRateParameter = (int) rate; }
  double getReleaseRate() const { return (double) releaseRateParameter.get(); }
  double getCalculatedReleaseRate() const { return (double) calcCoef(getReleaseRate(), targetRatioDR); }
  
  void setSustainLevel(double level) { sustainLevelParameter = (float) level; }
  double getSustainLevel() const { return (double) sustainLevelParameter.get(); }
  
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
      
      if (lock())
      {
        process();
        unlock();
      }
      
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
  void setTargetRatioA(double targetRatio) { targetRatioA = std::max(0.000000001, targetRatio); }
  
  double targetRatioDR;
  void setTargetRatioDR(double targetRatio) { targetRatioDR = std::max(0.000000001, targetRatio); }
  
  double calcCoef(double rate, double targetRatio) const {
    return (rate <= 0) ? 0.0 : exp(-log((1.0 + targetRatio) / targetRatio) / rate);
  }
};


#pragma mark - Beat

enum BeatType : int {
  BAR = 0,
  HALF = 1,
  QUARTER = 2,
  EIGHT = 3,
  SIXTEENTH = 4,
  THIRTYSECOND = 5
};

inline std::ostream& operator<<(std::ostream& os, const BeatType& type)
{
  switch (type) {
    case BeatType::BAR: os << "1/1 Bar"; break;
    case BeatType::HALF: os << "1/2 Bar"; break;
    case BeatType::QUARTER: os << "1/4 Bar"; break;
    case BeatType::EIGHT: os << "1/8 Bar"; break;
    case BeatType::SIXTEENTH: os << "1/16 Bar"; break;
    case BeatType::THIRTYSECOND: os << "1/32 Bar"; break;
    default: os << "1/4 Bar"; break;
  }
  return os;
}

static inline float getBeatLength(float BPM, BeatType type)
{
  static std::unordered_map<BeatType, float> multipliers = {
    { BeatType::BAR, 4.0 },
    { BeatType::HALF, 2.0 },
    { BeatType::QUARTER, 1.0 },
    { BeatType::EIGHT, 0.5 },
    { BeatType::SIXTEENTH, 0.25 },
    { BeatType::THIRTYSECOND, 0.125 }
  };
  
  float beatLength = 60.0 / BPM;
  return beatLength * multipliers.at(type);
}

struct BeatEvent : public ofEventArgs {
  bool isBar { false };
  bool isHalf { false };
  bool isQuarter { false };
  bool is8th { false };
  bool is16th { false };
  bool is32th { false };
  float BPM { 120 };
  
  bool operator[](int i) const {
    i = i % 5;
    switch (i) {
      case BeatType::BAR: return isBar;
      case BeatType::HALF: return isHalf;
      case BeatType::QUARTER: return isQuarter;
      case BeatType::EIGHT: return is8th;
      case BeatType::SIXTEENTH: return is16th;
      case BeatType::THIRTYSECOND: return is32th;
      default: return isQuarter;
    }
  }
  
  friend std::ostream& operator<<(std::ostream& os, const BeatEvent& beat)
  {
    os << (beat.isBar ? "●" : "○") << " " << (beat.isHalf ? "●" : "○") << " " << (beat.isQuarter ? "●" : "○") << " " << (beat.is8th ? "●" : "○") << " " << (beat.is16th ? "●" : "○") << " " << (beat.is32th ? "●" : "○");
    return os;
  }
};

class Beat {
public:
  Beat() {
    this->setBPM(120);
    
    parameters.setName("Beat");
    
    parameters.add(BPM.set("BPM", 120, 60, 280));
    BPM.addListener(this, &Beat::setBPMInternal);
    
    parameters.add(resetBPM);
    resetBPM.addListener(this, &Beat::reset);
    
    parameters.add(tapTrigger);
    tapTrigger.addListener(this, &Beat::trigger);
    
    ofAddListener(ofEvents().update, this, &Beat::updateHandler);
  }
  
  ~Beat() {
    BPM.removeListener(this, &Beat::setBPMInternal);
    resetBPM.removeListener(this, &Beat::reset);
    tapTrigger.removeListener(this, &Beat::trigger);
    
    ofRemoveListener(ofEvents().update, this, &Beat::updateHandler);
  }
  
  void reset()
  {
    lastTriggerTime = ofGetElapsedTimeMillis();
    beat = 0;
  }
  
  void setBPM(float bpm)
  {
    this->BPM.setWithoutEventNotifications(bpm);
//    timer.setPeriodicEvent(BPMtoNano(this->BPM * 32 / 4.0));
    reset();
  }
  
  double getBPM() const { return BPM; }
  
  float getBeatLength(int division) const
  {
    return ((60.0 / getBPM()) * 4.0) / pow(2, division % 5);
  }
  
  static int size() { return 5; }
  
  ofEvent<BeatEvent> onBeatE;
  ofParameterGroup parameters;
  
protected:
  unsigned long beat { 0 };
  
  uint64_t lastTriggerTime { ofGetElapsedTimeMillis() };
  
  ofParameter<void> resetBPM { "Reset BPM" };
  ofParameter<int> BPM { "BPM", 120, 60, 200 };
  void setBPMInternal(int & param) { this->setBPM(param); }
  
  ofParameter<void> tapTrigger { "Tap BPM" };
  uint64_t lastTapTime { ofGetElapsedTimeMillis() };
  std::deque<float> differences;
  void trigger()
  {
    float time = ofGetElapsedTimeMillis();
    float diff = time - lastTapTime; // Difference in milliseconds
    
    if (diff > 10.0f * 1000) differences.clear();
    
    differences.push_back(diff);
    
    if (differences.size() > 12)
    {
      differences.pop_front();
      
      float BPM = 60.0 / (Array::average(differences) / 1000);
      this->setBPM(BPM);
    }
    
    lastTapTime = time;
  }
  
  uint64_t BPMtoNano(double bpm)
  {
    double seconds = 60.0 / bpm;
    return seconds * 1000000000;
  }
  
  uint64_t BPMtoMillis(double bpm)
  {
    double seconds = 60.0 / bpm;
    return seconds * 1000;
  }
  
  void updateHandler(ofEventArgs & e) {
    uint64_t currentTime = ofGetElapsedTimeMillis();
    
    if (currentTime - lastTriggerTime >= BPMtoMillis(this->BPM * 32 / 4.0))
    {
      beat = (beat + 1) % 32;
      
      BeatEvent data;
      
      data.isBar = (beat & 31) == 0;      // 32-beat interval
      data.isHalf = (beat & 15) == 0;     // 16-beat interval
      data.isQuarter = (beat & 7) == 0;   // 8-beat interval
      data.is8th = (beat & 3) == 0;       // 4-beat interval
      data.is16th = (beat & 1) == 0;      // 2-beat interval
      data.is32th = true;                 // Always true
      
//      std::cout << "Beat Sync: BEAT! " << data << " Interval = " << BPMtoMillis(this->BPM * 32 / 4.0) << std::endl;
      onBeatE.notify(data);
      lastTriggerTime = currentTime;
    }
  }
};

class BeatAsync : public ofThread {
public:
  BeatAsync() {
    this->setBPM(120);
    
    parameters.setName("Beat");
    
    parameters.add(BPM.set("BPM", 120, 60, 280));
    BPM.addListener(this, &BeatAsync::setBPMInternal);
    
    parameters.add(resetBPM);
    resetBPM.addListener(this, &BeatAsync::reset);
    
    parameters.add(tapTrigger);
    tapTrigger.addListener(this, &BeatAsync::trigger);
    
    startThread();
  }
  
  ~BeatAsync() {
    BPM.removeListener(this, &BeatAsync::setBPMInternal);
    resetBPM.removeListener(this, &BeatAsync::reset);
    tapTrigger.removeListener(this, &BeatAsync::trigger);
    
    waitForThread(true);
  }
  
  void reset()
  {
    beat = 0;
    timer.reset();
  }
  
  void setBPM(float bpm)
  {
    this->BPM.setWithoutEventNotifications(bpm);
    timer.setPeriodicEvent(BPMtoNano(this->BPM * 32 / 4.0));
    reset();
  }
  
  double getBPM() const { return BPM; }
  
  float getBeatLength(int division) const
  {
    return ((60.0 / getBPM()) * 4.0) / pow(2, division % 5);
  }
  
  static int size() { return 5; }
  
  ofEvent<BeatEvent> onBeatE;
  ofParameterGroup parameters;
  
protected:
  unsigned long beat { 0 };
  
  ofTimer timer;
  
  ofParameter<void> resetBPM { "Reset BPM" };
  ofParameter<int> BPM { "BPM", 120, 60, 200 };
  void setBPMInternal(int & param) { this->setBPM(param); }
  
  ofParameter<void> tapTrigger { "Tap BPM" };
  uint64_t lastTapTime { ofGetElapsedTimeMillis() };
  std::deque<float> differences;
  void trigger()
  {
    float time = ofGetElapsedTimeMillis();
    float diff = time - lastTapTime; // Difference in milliseconds
    
    if (diff > 10.0f * 1000) differences.clear();
    
    differences.push_back(diff);
    
    if (differences.size() > 12)
    {
      differences.pop_front();
      
      float BPM = 60.0 / (Array::average(differences) / 1000);
      this->setBPM(BPM);
    }
    
    lastTapTime = time;
  }
  
  uint64_t BPMtoNano(double bpm)
  {
    double seconds = 60.0 / bpm;
    return seconds * 1000000000;
  }
  
  void threadedFunction() {
    uint64_t currentTime = ofGetElapsedTimeMillis();
    
    while (isThreadRunning()) 
    {
      if (lock())
      {
        beat = (beat + 1) % 32;
        
        BeatEvent data;
        data.isBar = (beat & 31) == 0;      // 32-beat interval
        data.isHalf = (beat & 15) == 0;     // 16-beat interval
        data.isQuarter = (beat & 7) == 0;   // 8-beat interval
        data.is8th = (beat & 3) == 0;       // 4-beat interval
        data.is16th = (beat & 1) == 0;      // 2-beat interval
        data.is32th = true;                 // Always true
        
        onBeatE.notify(data);
        
        unlock();
      }
      
      timer.waitNext();
    }
  }
};

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
