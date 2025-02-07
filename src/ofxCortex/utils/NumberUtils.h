#pragma once

#include "ofAppRunner.h"
#include "ofParameter.h"

namespace ofxCortex { namespace core { namespace utils {

inline static float roundToNearest(float value, float multiple)
{
  if (multiple == 0)
    return value;
  
  int remainder = fmod(abs(value), multiple);
  if (remainder == 0)
    return value;
  
  if (value < 0)
    return -(abs(value) - remainder);
  else
    return value + multiple - remainder;
};

inline static double floorToNearest(double value, double multiple)
{
  double divider = 1.0 / multiple;
  return floor(value * divider) / divider;
}

inline static unsigned long modulo(int a, int b) { return (b + (a % b)) % b; }

//template <typename T>
//T wrappedDistance(T a, T b, T length) {
//    static_assert(std::is_arithmetic<T>::value, "wrappedDistance requires a numeric type");
//    
//    T dist = std::fmod(b - a + length / T(2), length) - length / T(2);
//    if (dist < T(0)) {
//        dist += length;
//    }
//    return dist;
//}

//template<typename T>
//static T wrappedDistance(T a, T b, T length)
//{
//  return modulo(a - b, length);
//}

template<typename T>
static T wrappedDistance(T a, T b, T length) {
  // Calculate the direct distance (clockwise)
  int directDistance = std::abs((int)(b - a));
  
  // Calculate the wrapped distance (counterclockwise)
  int wrappedDistance = length - directDistance;
  
  // Return the minimum of the two distances
  return std::min(directDistance, wrappedDistance);
}

template<typename T>
inline static double normalizeIndex(size_t index, const std::vector<T> & v) { return (double) index / (v.size() - 1); }

template <typename T>
inline static T gradientDescent(T initialValue, std::function<T(T)> gradientFunc, double learningRate, T tolerance, int maxIterations) {
  T value = initialValue;  // Start with the initial value
  int iterations = 0;
  
  while (std::abs(gradientFunc(value)) > tolerance && iterations < maxIterations) {
    T gradient = gradientFunc(value);  // Calculate gradient, often the difference between the current value and the target value
    value = value - learningRate * gradient;  // Update value
    
    iterations++;
  }
  
  return value;
}

template<typename T>
inline static void generatePartitions(const std::vector<T>& set, size_t idx, std::vector<std::set<T>>& currentPartition, std::vector<std::vector<std::set<T>>>& allPartitions) {
    if (idx == set.size()) {
        // When all elements have been assigned, store the current partition
        allPartitions.push_back(currentPartition);
        return;
    }

    // Try to add the current element (set[idx]) to each subset in currentPartition
    size_t n = currentPartition.size();
    for (size_t i = 0; i < n; ++i) {
        currentPartition[i].insert(set[idx]);
        generatePartitions(set, idx + 1, currentPartition, allPartitions);
        currentPartition[i].erase(set[idx]);  // Backtrack
    }

    // Or create a new subset with the current element
    currentPartition.push_back({set[idx]});
    generatePartitions(set, idx + 1, currentPartition, allPartitions);
    currentPartition.pop_back();  // Backtrack
}

template<typename T>
inline static std::vector<std::vector<std::set<T>>> getPartitions(const std::vector<T>& set)
{
  std::vector<std::vector<std::set<T>>> allPartitions;
  
  std::vector<std::set<T>> currentPartition;
  generatePartitions(set, 0, currentPartition, allPartitions);
  
  return allPartitions;
}

template <typename T>
std::vector<std::vector<std::pair<T, T>>> getUniquePairs(std::vector<T> input)
{
  std::vector<std::vector<std::pair<T, T>>> result;
  
  if (input.empty()) return result;
  
  // If the size of input is odd, it's impossible to form distinct pairs
  if (input.size() % 2 != 0) {
    ofLogWarning("ofxCortex::core::utils::getUniquePairs") << "Input size is not even.";
    return result;
  }
  
  std::sort(input.begin(), input.end());
  
  // Generate all permutations
  do {
    std::vector<std::pair<T, T>> currentPairs;
    
    // Form pairs from the permutation
    size_t i = 0;
    for (; i + 1 < input.size(); i += 2) {
      currentPairs.emplace_back(input[i], input[i + 1]);
    }
    
    // Handle the odd unpaired element
    if (i < input.size()) {
      // Add the last unpaired element in a way that the caller can recognize
      currentPairs.emplace_back(input[i], T{}); // T{} represents the unpaired element
    }
    
    result.push_back(currentPairs);
    
  } while (std::next_permutation(input.begin(), input.end()));
  
  return result;
}

template <typename T>
void backtrackPairs(
    const std::vector<T>& input,
    std::vector<std::pair<T, T>>& currentPairs,
    std::vector<std::vector<std::pair<T, T>>>& results,
    std::vector<bool>& used,
    int unpairedIdx = -1) {

    // Base case: all elements are used
    if (currentPairs.size() * 2 + (unpairedIdx != -1 ? 1 : 0) == input.size()) {
        results.push_back(currentPairs);
        return;
    }

    // Find the first unused element
    int firstUnused = -1;
    for (size_t i = 0; i < input.size(); ++i) {
        if (!used[i]) {
            firstUnused = i;
            break;
        }
    }

    // Handle unpaired element if needed
    if (unpairedIdx == -1 && input.size() % 2 != 0) {
        unpairedIdx = firstUnused;
        used[firstUnused] = true;
        backtrackPairs(input, currentPairs, results, used, unpairedIdx);
        used[firstUnused] = false;
        return;
    }

    // Pair the first unused element with every other unused element
    used[firstUnused] = true;
    for (size_t j = firstUnused + 1; j < input.size(); ++j) {
        if (!used[j]) {
            // Pair firstUnused and j
            currentPairs.emplace_back(input[firstUnused], input[j]);
            used[j] = true;

            // Recurse
            backtrackPairs(input, currentPairs, results, used, unpairedIdx);

            // Backtrack
            currentPairs.pop_back();
            used[j] = false;
        }
    }
    used[firstUnused] = false;
}

// Wrapper function
template <typename T>
inline static std::vector<std::vector<std::pair<T, T>>> generatePairCombinations(const std::vector<T>& input) {
    std::vector<std::vector<std::pair<T, T>>> results;
    std::vector<std::pair<T, T>> currentPairs;
    std::vector<bool> used(input.size(), false);

    backtrackPairs(input, currentPairs, results, used);
    return results;
}

class TimeIncrementer {
public:
  TimeIncrementer(const std::string & name = "Time", float min = -10.0, float max = 10.0) : value(0.0)
  {
    ofAddListener(ofEvents().update, this, &TimeIncrementer::update);
    
    parameters.setName(name);
    parameters.add(currentValue, speed);
    speed.setMin(min);
    speed.setMax(max);
  }
  
  ~TimeIncrementer() { ofRemoveListener(ofEvents().update, this, &TimeIncrementer::update); }
  
  operator float() const { return value; }
  operator ofParameterGroup&() { return parameters; }
  
  ofParameterGroup parameters;
protected:
  float value;
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> speed { "Speed", 0.1, -10.0, 10.0 };
  
  void update(ofEventArgs & e) { value += ofGetLastFrameTime() * speed; currentValue.set(ofToString(value)); }
};

template<typename T = float>
class Lerped {
public:
  Lerped(T value = T(), float _smoothing = 0.1f, const std::string & name = "Lerped Value") : current(value), target(value)
  {
    ofAddListener(ofEvents().update, this, &Lerped::update);
    
    parameters.setName(name);
    parameters.add(currentValue, smoothing);
    this->smoothing.set(_smoothing);
  }
  
  ~Lerped()
  {
    ofRemoveListener(ofEvents().update, this, &Lerped::update);
  }
  
  void operator=(const T& value) { this->setTarget(value); }
  operator T() const { return current; }
  operator ofParameterGroup&() { return parameters; }
  
  void setTarget(const T & value) { this->target = value; }
  void setCurrent(const T & value) { this->current = value; }
  void setSmoothing(float value) { this->smoothing = value; }
  
  
  ofParameterGroup parameters;
protected:
  T current;
  T target;
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> smoothing { "Smoothing", 0.1, 0.0, 1.0 };
  
  void update(ofEventArgs & e) {
    if (ofIsFloatEqual(current, target)) { current = target; return; }
    
    current = ofLerp(current, target, 1.0 - exp(-smoothing.get() * ofGetLastFrameTime()));
    currentValue = ofToString(current);
  }
};

template<>
inline void Lerped<glm::vec3>::update(ofEventArgs & e)
{
  if (current == target) return;
  
  current = glm::mix(current, target, 1.0 - exp(-smoothing.get() * ofGetLastFrameTime()));
}



template<typename T>
class InertialLerp {
public:
  InertialLerp(T value = T(), const std::string & name = "Lerped Value") : current(value), previousTarget(value), target(value), velocity(0)
  {
    ofAddListener(ofEvents().update, this, &InertialLerp::update);
    
    parameters.setName(name);
    parameters.add(currentValue, stiffness, damping, anticipation);
  }
  
  ~InertialLerp()
  {
    ofRemoveListener(ofEvents().update, this, &InertialLerp::update);
  }
  
  void operator=(const T& value) { this->setTarget(value); }
  operator T() const { return current; }
  operator ofParameterGroup&() { return parameters; }
  
  void setTarget(const T & value) { this->target = value; }
  void setCurrent(const T & value) { this->current = value; }
  
  InertialLerp<T> & setStiffness(float value) { this->stiffness = value; return *this; }
  InertialLerp<T> & setDamping(float value) { this->damping = value; return *this; }
  InertialLerp<T> & setAnticipation(float value) { this->anticipation = value; return *this; }
  
  
  ofParameterGroup parameters;
protected:
  T target; // x
  T previousTarget; // xp
  
  T current; // y
  T velocity; // yd
  
  ofParameter<std::string> currentValue { "Value", "0.0" };
  ofParameter<float> stiffness { "Stiffness", 4.0, 0.0, 10.0 }; // f
  ofParameter<float> damping { "Damping", 0.25, 0.0, 1.0 }; // zeta
  ofParameter<float> anticipation { "Anticipation", 0., -2.0, 2.0 }; // r
  
  bool isEqual() const { return current == target; }
  
  void update(ofEventArgs & e) {
    if (isEqual()) return;
    
    const float & f = stiffness.get();
    const float & z = damping.get();
    const float & r = anticipation.get();
    
    float k1 = z / (PI * f);
    float k2 = 1.0 / ((TWO_PI * f) * (TWO_PI * f));
    float k3 = r * z / (TWO_PI * f);
    
    float delta = ofGetLastFrameTime();
    T xd = (target - previousTarget) / delta;
    previousTarget = target;
    
    float k2_stable = std::max(k2, 1.1f * (delta * delta / 4.0f + delta * k1 / 2.0f));
    
    current = current + delta * velocity;
    velocity = velocity + delta * (target + k3 * xd - current - k1 * velocity) / k2_stable;
    
    currentValue = ofToString(current);
  }
};

//template<> bool InertialLerp<float>::isEqual() const { return ofIsFloatEqual(current, target); }

}}}
