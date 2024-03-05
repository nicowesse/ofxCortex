#pragma once

#include <functional>
#include <chrono>
#include <utility>

#include "ofUtils.h"
#include "ofTimer.h"
#include "ofThread.h"

namespace ofxCortex::core {

template<typename T>
class Timer : public ofThread {
public:
  Timer() {}
  ~Timer() {
    waitForThread(true, 1000);
  }
  
//  template<typename Func>
  static std::shared_ptr<Timer> setTimer(std::function<void(T)> && callback, uint64_t intervalMs, bool cyclic = false, bool autostart = true)
  {
    static uint64_t nanosecondToMs = 1000000;
    
    auto instance = std::make_shared<Timer>();
    instance->callback = std::forward<std::function<void(T)>>(callback);
    instance->cyclic = cyclic;
    
    instance->timer.setPeriodicEvent(nanosecondToMs * intervalMs);
    if (autostart) instance->reset();
    
    return instance;
  }
  
  static std::shared_ptr<Timer> setInterval(std::function<void(T)> && callback, uint64_t intervalMs, bool autostart = true)
  {
    return std::move(Timer::setTimer(std::forward<std::function<void(T)>>(callback), intervalMs, true, autostart));
  }
  
  static std::shared_ptr<Timer> setTimeout(std::function<void(T)> && callback, uint64_t intervalMs, bool autostart = true)
  {
    return Timer::setTimer(std::forward<std::function<void(T)>>(callback), intervalMs, false, autostart);
  }
  
  void reset() {
    timer.reset();
    this->startThread();
  }
  
protected:
  ofTimer timer;
  bool cyclic { false };
  std::function<void(T)> callback;
  
  void threadedFunction() {
    while(isThreadRunning()) {
      timer.waitNext();
      
      this->callback();
      
      if (!this->cyclic) stopThread();
    }
  }
};

template<>
class Timer<void> : public ofThread {
public:
  Timer() {}
  ~Timer() {
    waitForThread(true, 1000);
  }
  
  static std::shared_ptr<Timer> setTimer(std::function<void()> && callback, uint64_t intervalMs, bool cyclic = false, bool autostart = true)
  {
    static uint64_t nanosecondToMs = 1000000;
    
    auto instance = std::make_shared<Timer>();
    instance->callback = std::forward<std::function<void()>>(callback);
    instance->cyclic = cyclic;
    
    instance->timer.setPeriodicEvent(nanosecondToMs * intervalMs);
    if (autostart) instance->reset();
    
    return instance;
  }
  
  static std::shared_ptr<Timer> setInterval(std::function<void()> && callback, uint64_t intervalMs, bool autostart = true)
  {
    return Timer::setTimer(std::forward<std::function<void()>>(callback), intervalMs, true, autostart);
  }
  
  static std::shared_ptr<Timer> setTimeout(std::function<void()> && callback, uint64_t intervalMs, bool autostart = true)
  {
    return Timer::setTimer(std::forward<std::function<void()>>(callback), intervalMs, false, autostart);
  }
  
  void reset() {
    timer.reset();
    this->startThread();
  }
  
protected:
  ofTimer timer;
  bool cyclic { false };
  std::function<void()> callback;
  
  void threadedFunction() {
    while(isThreadRunning()) {
      timer.waitNext();
      
      this->callback();
      
      if (!this->cyclic) stopThread();
    }
  }
};

class Timing {
public:
  Timing(const Timing&) = delete;
  Timing& operator=(const Timing &) = delete;
  Timing(Timing &&) = delete;
  Timing & operator=(Timing &&) = delete;
  
  
  template<typename Func>
  static std::shared_ptr<Timer<void>> setInterval(Func && callback, uint64_t intervalMs, bool autostart = true)
  {
    auto timer = Timer<void>::setTimer(std::forward<Func>(callback), intervalMs, true, autostart);
    Timing::instance().pool.push_back(timer);
    
    return timer;
  }
  
  template<typename Func>
  static std::shared_ptr<Timer<void>> setTimeout(Func && callback, uint64_t intervalMs, bool autostart = true)
  {
    auto timer = Timer<void>::setTimer(std::forward<Func>(callback), intervalMs, false, autostart);
    Timing::instance().pool.push_back(timer);
    
    return timer;
  }
  
private:
  Timing() {}; // Disallow instantiation outside of the class.
  ~Timing() {};
  
  static Timing& instance(){
    static Timing instance;
    return instance;
  }
  
  std::vector<std::shared_ptr<Timer<void>>> pool;
};

};
