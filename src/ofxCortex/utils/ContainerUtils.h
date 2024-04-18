#pragma once

#include <functional>
#include <algorithm>
#include <type_traits>

#include "ofxCortex/utils/NumberUtils.h"

namespace ofxCortex { namespace core { namespace utils {

#define ALIAS_TEMPLATE_FUNCTION(highLevelF, lowLevelF) \
template<typename... Args> \
inline auto highLevelF(Args&&... args) -> decltype(lowLevelF(std::forward<Args>(args)...)) \
{ \
return lowLevelF(std::forward<Args>(args)...); \
}

namespace Array {

template<typename Container, class T = typename Container::value_type>
static T& atWrapped(Container & v, int index)
{
  return v[modulo(index, v.size())];
}

template<typename Container, class T = typename Container::value_type>
static T randomInVector(const Container & v)
{
  auto it = v.cbegin();
  int random = rand() % v.size();
  std::advance(it, random);
  
  return *it;
}

ALIAS_TEMPLATE_FUNCTION(sample, randomInVector)

template<typename T>
T randomWeighted(const std::unordered_map<T, float>& weightedMap) 
{
  std::random_device rd;
  std::mt19937 gen(rd());
  
  std::vector<T> elements;
  std::vector<float> probabilities;
  for (const auto& pair : weightedMap) {
    elements.push_back(pair.first);
    probabilities.push_back(pair.second);
  }
  
  std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());
  return elements[distribution(gen)];
}

template<typename Container, class T = typename Container::value_type>
Container exclude(const Container& input, const Container& exclude)
{
  Container result;
  std::copy_if(input.cbegin(), input.cend(), std::back_inserter(result), [&](const T& element) {
    return std::find(exclude.cbegin(), exclude.cend(), element) == exclude.cend();
  });
  return result;
}

template<typename Container, class T = typename Container::value_type>
static void remove(Container& source, const Container& remove)
{
  source.erase(std::remove_if(source.begin(), source.end(), [&remove](const T& item) {
    return std::find(remove.cbegin(), remove.cend(), item) != remove.cend();
  }), source.end());
}

template<typename Container, class T = typename Container::value_type>
static T randomInVectorExcept(const Container& v, const Container& except) { return randomInVector(exclude(v, except)); }

ALIAS_TEMPLATE_FUNCTION(sampleExcept, randomInVectorExcept)

template<typename Container, class T = typename Container::value_type>
static T sampleExcept(const Container & v, const T& except)
{
  return randomInVectorExcept(v, { except });
}

template<typename Container, class T = typename Container::value_type>
void subtractFromVector(Container& input, const Container& subtract)
{
  input.erase(std::remove_if(input.begin(), input.end(), [&subtract](T element) { return std::find(subtract.begin(), subtract.end(), element) != subtract.end(); }), input.end());
}

template<typename T>
static std::vector<T> constructVector(int n, std::function<T(int)> func = [](int index) { return T(); })
{
  std::vector<T> output(n);
  for (int i = 0; i < n; i++) output[i] = func(i);
  return output;
}

template<typename T>
static std::vector<T> constructVector(int n, std::function<T(int, float)> func = [](int index, float t) { return T(); })
{
  std::vector<T> output(n);
  for (int i = 0; i < n; i++) output[i] = func(i, (float) i / (n - 1.0f));
  return output;
}

template<typename T>
static std::vector<T> constructVector(int columns, int rows, std::function<T(int, int)> func = [](int col, int row) { return T(); })
{
  std::vector<T> output(columns * rows);
  for (int row = 0; row < rows; row++)
  {
    for (int column = 0; column < columns; column++) { output[column + row * columns] = func(column, row); }
  }
  return output;
}

template<typename T>
static std::vector<T> flatten(const std::vector<std::vector<T>> & v)
{
  std::vector<T> output;
  for(const auto & x : v)
    output.insert(output.end(), x.begin(), x.end());
  return output;
}

template<typename T>
static int findIndex(const std::vector<T> & v, const T & needle)
{
  auto it = find(begin(v), end(v), needle);
  return (it != v.end()) ? it - v.begin() : -1;
}

template<typename T>
static std::vector<unsigned int> findIndices(const std::vector<T> & v, const std::vector<T> & needles)
{
  std::vector<unsigned int> indices;
  for (const T & needle : needles)
  {
    int index = findIndex(v, needle);
    if (index != -1) indices.push_back(index);
  }
  return indices;
}

template <typename T>
static std::vector<T> randomSubset(const std::vector<T>& v, std::size_t size)
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

ALIAS_TEMPLATE_FUNCTION(sampleN, randomSubset)

template<typename T>
std::vector<size_t> randomIndices(const std::vector<T>& inputVector, size_t n = -1) {
  if (n == 0 || inputVector.size() == 0) return std::vector<size_t>();
  else if (n == -1) n = inputVector.size();
  else n = std::min(n, inputVector.size()); // Clamp n to the size of the input vector
  
  std::vector<size_t> indices(inputVector.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  
  std::shuffle(indices.begin(), indices.end(), gen);
  
  return std::vector<size_t>(indices.begin(), indices.begin() + n);
}

//template<typename T>
template<typename T, template<typename, typename> class Container, typename Allocator = std::allocator<T>>
static void appendVector(Container<T, Allocator> & original, Container<T, Allocator> & appending)
{
  original.insert(std::end(original), std::begin(appending), std::end(appending));
}

ALIAS_TEMPLATE_FUNCTION(append, appendVector)

template<typename OutputType, typename InputType, typename Func>
static std::vector<OutputType> transform(const std::vector<InputType> & v, Func func)
{
  std::vector<OutputType> output;
  std::transform(begin(v), end(v), std::back_inserter(output), func);
  return output;
}

ALIAS_TEMPLATE_FUNCTION(map, transform)

//template<typename T, template <typename, typename...> class Container, typename... Args, typename Func, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
//static std::vector<T> filter(const Container<T, Args...> & v, Func func)
//{
//  std::vector<T> output;
//  std::copy_if(begin(v), end(v), std::back_inserter(output), func);
//  return output;
//}

template <typename Container, typename Function>
static Container filter(const Container& source, const Function & func)
{
  Container output;
  std::copy_if(source.begin(), source.end(), std::inserter(output, output.end()), func);
  return output;
}

//template<typename InputType, typename OutputType, typename Func>
//static OutputType accumulate(const std::vector<InputType> & v, Func func, OutputType initialValue = OutputType(0))
//{
//  return std::accumulate(begin(v), end(v), initialValue, func);
//}

//template <
//  typename InputType,
//  typename OutputType = InputType,
//  template <typename, typename...> class Container,
//  typename... Args,
//  typename Func = std::plus<InputType>,
//  typename = typename std::enable_if<std::is_arithmetic<InputType>::value, InputType>::type
//>
//static OutputType accumulate(const Container<InputType, Args...>& container, Func func = Func())
//{
//    return std::accumulate(container.begin(), container.end(), OutputType(), func);
//}

template <
typename OutputType,
typename Container,
typename Func
>
static OutputType accumulate(const Container& container, Func func = Func(), OutputType initial = OutputType())
{
  return std::accumulate(container.begin(), container.end(), initial, func);
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static std::vector<T> normalize(const std::vector<T> & v)
{
  T total = accumulate<T>(v, [](T carry, T x) { return carry + x; });
  return transform<T, T>(v, [&](T x) { return x / total; });
}

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static T average(const Container<T, Args...> & v)
{
  return accumulate<T>(v, [](T carry, T x) { return carry + x; }) / v.size();
}

template<typename T, template <typename, typename...> class Container, typename... Args, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
static T median(const Container<T, Args...> & v)
{
  auto sorted = v;
  std::sort(begin(sorted), end(sorted));
  
  size_t size = v.size();
  
  if (size % 2 == 0) return (v[(size - 1) / 2] + v[size / 2]) / 2.0;
  else return v[size / 2];
}

template<typename T>
static std::vector<T> intersection(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::vector<T> output;
  std::set_intersection(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(output));
  return output;
}

template<typename T>
static std::vector<T> difference(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> output;
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  std::set_difference(A.begin(), A.end(), B.begin(), B.end(), std::back_inserter(output));
  return output;
}

template<typename T>
static bool includes(const std::vector<T> & a, const std::vector<T> & b)
{
  std::vector<T> A = a;
  std::vector<T> B = b;
  
  std::sort(A.begin(), A.end());
  std::sort(B.begin(), B.end());
  
  return std::includes(A.begin(), A.end(), B.begin(), B.end());
}

enum CoordinateEdges {
  WRAP, CLAMP, CRASH
};

static unsigned int coordinateToIndex(glm::ivec3 coord, glm::ivec3 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x + modulo(coord.z, size.z) * size.x * size.y;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x + CLAMP(coord.z, 0, size.z - 1) * size.x * size.y;
  else return coord.x + coord.y * size.x + coord.z * size.x * size.y;
}
static unsigned int coordinateToIndex(glm::ivec2 coord, glm::ivec2 size, CoordinateEdges edgeMode = CLAMP) {
  if (edgeMode == WRAP) return modulo(coord.x, size.x) + modulo(coord.y, size.y) * size.x;
  else if (edgeMode == CLAMP) return CLAMP(coord.x, 0, size.x - 1) + CLAMP(coord.y, 0, size.y - 1) * size.x;
  else return coord.x + coord.y * size.x;
}

static glm::ivec3 indexToCoordinate(int index, glm::ivec3 size)
{
  if (index < 0 || index >= size.x * size.y * size.z) { index = CLAMP(index, 0, size.x * size.y * size.z - 1); }
  
  glm::ivec3 coord;
  coord.x = index % size.x;
  coord.y = (index / size.x) % size.y;
  coord.z = index / (size.x * size.y);
  
  return coord;
}

static glm::ivec2 indexToCoordinate(int index, glm::ivec2 size)
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
std::vector<Key> keys(const Map& m)
{
  std::vector<Key> keys;
  for (const auto& pair : m) {
    keys.push_back(pair.first);
  }
  return keys;
}

template<typename Map, typename Value = typename Map::key_type>
std::vector<Value> values(const Map& m)
{
  std::vector<Value> keys;
  for (const auto& pair : m) {
    keys.push_back(pair.second);
  }
  return keys;
}

}

}}}
