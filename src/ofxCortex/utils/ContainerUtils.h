#pragma once

#include <functional>
#include <algorithm>
#include <type_traits>
#include <unordered_set>

#include "ofxCortex/utils/NumberUtils.h"

namespace ofxCortex { namespace core { namespace utils {

#define ALIAS(highLevelF, lowLevelF) \
template<typename... Args> \
inline auto highLevelF(Args&&... args) -> decltype(lowLevelF(std::forward<Args>(args)...)) \
{ \
return lowLevelF(std::forward<Args>(args)...); \
}

namespace Array {

#pragma mark - Getters
template<typename Container, class T = typename Container::value_type>
inline static T& atWrapped(Container & v, int index)
{
  return v[modulo(index, v.size())];
}

#pragma mark - Vector Exclusion
template<typename Container, class T = typename Container::value_type>
inline static Container exclude(const Container& input, const Container& exclude)
{
  Container result;
  std::copy_if(input.cbegin(), input.cend(), std::back_inserter(result), [&](const T& element) {
    return std::find(exclude.cbegin(), exclude.cend(), element) == exclude.cend();
  });
  return result;
}

template<typename Container, class T = typename Container::value_type>
inline static void remove(Container& source, const Container& remove)
{
  source.erase(std::remove_if(source.begin(), source.end(), [&remove](const T& item) {
    return std::find(remove.cbegin(), remove.cend(), item) != remove.cend();
  }), source.end());
}

template<typename Container, class T = typename Container::value_type>
inline static void subtractFromVector(Container& input, const Container& subtract)
{
  input.erase(std::remove_if(input.begin(), input.end(), [&subtract](T element) { return std::find(subtract.begin(), subtract.end(), element) != subtract.end(); }), input.end());
}


#pragma mark - Vector Construction

template<typename T>
inline static std::vector<T> constructVector(int n, std::function<T(int)> func = [](int index) { return T(); })
{
  std::vector<T> output(n);
  for (int i = 0; i < n; i++) output[i] = func(i);
  return output;
}

template<typename T>
inline static std::vector<T> constructVector(int n, std::function<T(int, float)> func = [](int index, float t) { return T(); })
{
  std::vector<T> output(n);
  for (int i = 0; i < n; i++) output[i] = func(i, (float) i / (n - 0.0f));
  return output;
}

template<typename T>
inline static std::vector<T> constructVector(int columns, int rows, std::function<T(int, int)> func = [](int col, int row) { return T(); })
{
  std::vector<T> output(columns * rows);
  for (int row = 0; row < rows; row++)
  {
    for (int column = 0; column < columns; column++) { output[column + row * columns] = func(column, row); }
  }
  return output;
}

template<typename T>
inline static std::vector<T> flatten(const std::vector<std::vector<T>> & v)
{
  std::vector<T> output;
  for(const auto & x : v)
    output.insert(output.end(), x.begin(), x.end());
  return output;
}


#pragma mark - Indices

template<typename T>
inline static size_t findIndex(const std::vector<T> & v, const T & needle)
{
  auto it = find(begin(v), end(v), needle);
  return (it != v.end()) ? it - v.begin() : -1;
}

template<typename T, typename Func>
inline static size_t findIndex(const std::vector<T> & v, Func needleFunc)
{
  auto it = find_if(begin(v), end(v), needleFunc);
  return (it != v.end()) ? it - v.begin() : -1;
}

template<typename T>
inline static std::vector<unsigned int> indices(const std::vector<T> & v)
{
  std::vector<unsigned int> indices(v.size());
  std::iota(indices.begin(), indices.end(), 0);
  return indices;
}

inline static std::vector<unsigned int> indices(unsigned int n)
{
  std::vector<unsigned int> indices(n);
  std::iota(indices.begin(), indices.end(), 0);
  return indices;
}

template<typename T>
inline static std::vector<unsigned int> findIndices(const std::vector<T> & v, const std::vector<T> & needles)
{
  std::vector<unsigned int> indices;
  for (const T & needle : needles)
  {
    int index = findIndex(v, needle);
    if (index != -1) indices.push_back(index);
  }
  return indices;
}


#pragma mark - Random

template<typename Container, class T = typename Container::value_type>
inline static T randomInVector(const Container & v)
{
  auto it = v.cbegin();
  int random = rand() % v.size();
  std::advance(it, random);
  
  return *it;
}

ALIAS(sample, randomInVector)

template<typename Container, class T = typename Container::value_type>
inline static T randomInVectorExcept(const Container& v, const Container& except) { return randomInVector(exclude(v, except)); }

template<typename Container, class T = typename Container::value_type>
inline static T sampleExcept(const Container & v, const T& except) { return randomInVectorExcept(v, { except }); }



template<typename Container, class T = typename Container::value_type>
inline static size_t randomIndex(const Container & v) { return rand() % v.size(); }

template<typename T>
inline static T randomWeighted(const std::unordered_map<T, float>& weightedMap)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  
  std::vector<T> elements;
  std::vector<float> probabilities;
  for (const auto& pair : weightedMap) {
    elements.push_back(pair.first);
    probabilities.push_back(pair.second);
  }
  
  std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());
  return elements[distribution(gen)];
}

template <typename T>
inline static std::vector<T> randomSubset(const std::vector<T>& v, std::size_t size)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::vector<std::size_t> indices(v.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::shuffle(indices.begin(), indices.end(), gen);
  std::partial_sort(indices.begin(), indices.begin() + size, indices.end(),
                    [&gen](std::size_t i, std::size_t j) { return gen() < gen(); });
  std::vector<T> result(size);
  std::transform(indices.begin(), indices.begin() + size, result.begin(),
                 [&v](std::size_t i) { return v[i]; });
  return result;
}

ALIAS(sampleN, randomSubset)

template<typename T>
inline static std::vector<size_t> randomIndices(const std::vector<T>& inputVector, size_t n = -1) 
{
  if (n == 0 || inputVector.size() == 0) return std::vector<size_t>();
  else if (n == -1) n = inputVector.size();
  else n = std::min(n, inputVector.size()); // Clamp n to the size of the input vector
  
  std::vector<size_t> indices(inputVector.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  static std::random_device rd;
  static std::mt19937 gen(rd());
  
  std::shuffle(indices.begin(), indices.end(), gen);
  
  return std::vector<size_t>(indices.begin(), indices.begin() + n);
}

template <typename T>
inline void shuffle(std::vector<T>& v)
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  
  std::shuffle(v.begin(), v.end(), gen);
}

template <typename T>
static std::vector<T> getShuffled(const std::vector<T>& v)
{
  std::vector<T> output(v);
  
  static std::random_device rd;
  static std::mt19937 gen(rd());
  
  std::shuffle(output.begin(), output.end(), gen);
  
  return output;
}


#pragma mark - Vector Appending

template<typename T, template<typename, typename> class Container, typename Allocator = std::allocator<T>>
inline static void appendVector(Container<T, Allocator>& original, Container<T, Allocator>& appending)
{
  original.insert(std::end(original), std::begin(appending), std::end(appending));
}

template<typename T, template<typename, typename> class Container, typename Allocator = std::allocator<T>>
inline static void appendVector(Container<T, Allocator>& original, Container<T, Allocator>&& appending)
{
  original.insert(std::end(original), std::make_move_iterator(std::begin(appending)), std::make_move_iterator(std::end(appending)));
}

ALIAS(append, appendVector)

template<typename T>
inline static std::vector<T> cut(const std::vector<T>& vec, float max = 1.0, float min = 0.0)
{
  min = std::max(min, 0.0f);
  max = std::min(max, 1.0f);
  
  if (vec.size() == 0 || (min == 0.0 && max == 1.0)) return vec;
  
  std::vector<T> output(vec.begin() + (vec.size() * min), vec.begin() + (vec.size() * max));
  return output;
}


#pragma mark - Transform

template<typename OutputType, typename InputType, template<typename...> class Container, typename Function>
inline static Container<OutputType> transform(const Container<InputType> & v, Function func)
{
  Container<OutputType> output;
  std::transform(v.begin(), v.end(), std::back_inserter(output), func);
  return output;
}

ALIAS(map, transform)

template<typename, typename = std::void_t<>>
struct has_multiplication_operator : std::false_type {};
template<typename T> struct has_multiplication_operator<T, std::void_t<decltype(std::declval<T&>() * std::declval<T&>())>> : std::true_type {};
template<typename T> inline constexpr bool has_multiplication_operator_v = has_multiplication_operator<T>::value;


template<typename OutputType, typename InputType, template<typename...> class Container, typename FilterFunction>
inline static Container<OutputType> transform_filter(const Container<InputType> & v, FilterFunction func)
{
  static_assert(has_multiplication_operator_v<InputType>, "InputType must support multiplication operator");
  
  Container<OutputType> output; output.reserve(v.size());

  std::vector<size_t> indices(v.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  std::transform(indices.begin(), indices.end(), std::back_inserter(output), [&v, &func](size_t index) { return v[index] * func(index / (double)(v.size() - 1)); });
  
  return output;
}

template <typename Container, typename Function>
inline static Container filter(const Container& source, const Function & func)
{
  Container output;
  std::copy_if(source.begin(), source.end(), std::inserter(output, output.end()), func);
  return output;
}

template <typename OutputType, typename Container, typename Function>
inline static OutputType accumulate(const Container& container, Function func = Function(), OutputType initial = OutputType())
{
  return std::accumulate(container.begin(), container.end(), initial, func);
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline static std::vector<T> normalize(const std::vector<T> & v)
{
  T total = accumulate<T>(v, [](T carry, T x) { return carry + x; });
  return transform<T, T>(v, [&](T x) { return x / total; });
}

template<typename T>
inline static std::vector<T> resample(const std::vector<T>& input, size_t targetSize) {
    if (targetSize == 0) throw std::invalid_argument("Target size must be greater than 0.");
    
    // If the input is empty, return an empty vector.
    if (input.empty()) return {};

    // If the new size is 1, just return the first element.
    if (targetSize == 1) return { input.front() };

    std::vector<T> output(targetSize);
    
    // Compute the scaling factor.
    // We use (input.size()-1) because we want to interpolate between elements.
    double scale = static_cast<double>(input.size() - 1) / (targetSize - 1);
    
    for (size_t i = 0; i < targetSize; ++i) {
        double pos = i * scale;
        size_t index = static_cast<size_t>(pos);
        double fraction = pos - index;
        
        // If we are not at the end of the vector, interpolate between index and index+1.
        if (index + 1 < input.size()) output[i] = static_cast<T>(input[index] * (1 - fraction) + input[index + 1] * fraction);
        else output[i] = input[index];  // Should only happen at the very end.
    }
    
    return output;
}


#pragma mark - Statistical Functions

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline static T average(const Container<T, Args...> & v)
{
  return accumulate<T>(v, [](T carry, T x) { return carry + x; }) / v.size();
}

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
inline static T median(const Container<T, Args...> & v)
{
  auto sorted = v;
  std::sort(sorted.begin(), sorted.end());
  
  size_t size = v.size();
  
  if (size % 2 == 0) return (v[(size - 1) / 2] + v[size / 2]) / 2.0;
  else return v[size / 2];
}


#pragma mark - Sets

template <typename T>
inline static void unique(std::vector<T> & vec)
{
  std::sort(vec.begin(), vec.end());
  auto newEnd = std::unique(vec.begin(), vec.end());
  vec.erase(newEnd, vec.end());
}

template <typename T>
inline static std::vector<T> getUniques(const std::vector<T>& vec)
{
  std::unordered_set<T> seen;
  std::vector<T> result;
  result.reserve(vec.size());
  
  for (const auto& val : vec) {
    if (seen.insert(val).second) {
      result.push_back(val);
    }
  }
  
  return result;
}

template<typename T>
inline static std::vector<T> set_union(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::vector<T> output;
  std::set_union(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::back_inserter(output));
  return output;
}

template<typename T>
inline static std::vector<T> intersection(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::vector<T> output;
  std::set_intersection(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::back_inserter(output));
  return output;
}

template<typename T>
inline static std::vector<T> difference(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::vector<T> output;
  std::set_difference(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::back_inserter(output));
  return output;
}

template<typename T>
inline static bool includes(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  return std::includes(A.cbegin(), A.cend(), B.cbegin(), B.cend());
}


#pragma mark - Coordinates
enum CoordinateEdges {
  WRAP, CLAMP, CRASH
};

inline static unsigned int coordinateToIndex(glm::ivec3 coord, glm::ivec3 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x + modulo(coord.z, size.z) * size.x * size.y;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x + CLAMP(coord.z, 0, size.z - 1) * size.x * size.y;
  else return coord.x + coord.y * size.x + coord.z * size.x * size.y;
}
inline static unsigned int coordinateToIndex(glm::ivec2 coord, glm::ivec2 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x;
  else return coord.x + coord.y * size.x;
}

inline static glm::ivec3 indexToCoordinate(int index, glm::ivec3 size)
{
  if (index < 0 || index >= size.x * size.y * size.z) { index = CLAMP(index, 0, size.x * size.y * size.z - 1); }
  
  glm::ivec3 coord;
  coord.x = index % size.x;
  coord.y = (index / size.x) % size.y;
  coord.z = index / (size.x * size.y);
  
  return coord;
}

inline static glm::ivec2 indexToCoordinate(int index, glm::ivec2 size)
{
  if (index < 0 || index >= size.x * size.y) { index = CLAMP(index, 0, size.x * size.y - 1); }
  
  glm::ivec2 coord;
  coord.x = index % size.x;
  coord.y = index / size.x;
  
  return coord;
}


}

#pragma mark - Map
namespace Map {

template<typename Map, typename Key = typename Map::key_type>
inline static std::vector<Key> keys(const Map& m)
{
  std::vector<Key> keys;
  for (const auto& [key, value] : m) {
    keys.push_back(key);
  }
  return keys;
}

template<typename Map, typename Value = typename Map::mapped_type>
inline static std::vector<Value> values(const Map& m)
{
  std::vector<Value> values;
  for (const auto& [key, value] : m) {
    values.push_back(value);
  }
  return values;
}

template <typename MapType, typename Func>
auto transform(const MapType& input, Func&& func)
{
    using KeyType   = typename MapType::key_type;
    using InputType = typename MapType::mapped_type;
    using Compare   = typename MapType::key_compare;
    using Alloc     = typename MapType::allocator_type;

    // Deduce the result of calling 'func' on a value of type InputType
    using OutputType = std::invoke_result_t<Func, const InputType&>;

    // Rebind the allocator to match (const KeyType, OutputType)
    using OutputAlloc = typename std::allocator_traits<Alloc>
        ::template rebind_alloc<std::pair<const KeyType, OutputType>>;

    // Our resulting map has the same key type, comparator, and a re-bound allocator.
    std::map<KeyType, OutputType, Compare, OutputAlloc> output;

    // Populate 'output' by transforming each (key, value) in 'input'
    for (auto const& [k, v] : input) {
        output.emplace(k, func(v));
    }
    return output;
}

template<typename Key, typename Value>
inline static std::map<Key, Value> zip(const std::vector<Key> & keys, const std::vector<Value> & values)
{
  std::map<Key, Value> output;
  for (int i = 0; i < MIN(keys.size(), values.size()); i++)
  {
    output.insert({ keys[i], values[i] });
  }
  return output;
}

template<typename Map, typename Key = typename Map::key_type, typename Value = typename Map::mapped_type>
inline static std::map<Key, Value> randomSubset(const Map& inputMap, std::size_t size)
{
  std::vector<Key> keys;
  for (const auto& pair : inputMap) {
    keys.push_back(pair.first);
  }
  
  // Randomly shuffle the keys
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(keys.begin(), keys.end(), gen);
  
  // Create the subset map
  std::map<Key, Value> subset;
  for (size_t i = 0; i < size && i < keys.size(); ++i) {
    subset[keys[i]] = inputMap.at(keys[i]);
  }
  
  return subset;
}

} // Map



}}}
