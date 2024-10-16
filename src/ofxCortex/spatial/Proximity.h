#pragma once

#include <vector>
#include "ofVectorMath.h"
#include "ofxCortex/utils/GraphicUtils.h"
#include "ofxCortex/utils/DebugUtils.h"

namespace ofxCortex { namespace core { namespace spatial {

template <class T>
class Proximity2D
{
public:
  friend class ProximityRenderer;
  
  Proximity2D() = default;
  
  void setup(std::function<glm::vec2(const T&)> _getPositionFunc, glm::ivec2 _bins = glm::ivec2(10, 10), glm::vec2 _position = glm::vec2(), glm::vec2 _size = glm::vec2(ofGetWidth(), ofGetHeight()))
  {
    binCount = _bins;
    boundsPosition = _position;
    boundsSize = _size;
    binSize = boundsSize / (glm::vec2) binCount;
    
    bins.resize(binCount.x * binCount.y);
    getPositionFunction = _getPositionFunc;
  }
  
  void update()
  {
    for (auto & bin : bins) { bin.clear(); }
    
    for (auto & obj : items)
    {
      int binIndex = getBinIndexFromPosition(getPositionFunction(*obj));
      if (isValidBin(binIndex)) bins[binIndex].push_back(obj);
    }
  };
  
  std::vector<std::shared_ptr<T>> getNearby(const glm::vec2 & position, float radius) const
  {
    if (!getPositionFunction)
    {
      ofLogWarning() << "No position function!";
      return std::vector<std::shared_ptr<T>>();
    }
    
    int minXBin = indexX(position.x - radius);
    int maxXBin = indexX(position.x + radius);
    int minYBin = indexY(position.y - radius);
    int maxYBin = indexY(position.y + radius);
    
    if (maxXBin < minXBin) std::swap(maxXBin, minXBin);
    if (maxYBin < minYBin) std::swap(maxYBin, minYBin);
    
    std::vector<std::shared_ptr<T>> neighbours;
    
    float maxDistance = radius * radius;
    
    for (int x = minXBin; x <= maxXBin; x++)
    {
      for (int y = minYBin; y <= maxYBin; y++)
      {
        int index = to1D(x, y);
        if (isValidBin(index))
        {
          for (const std::shared_ptr<T> & item : bins[index])
          {
            float d = glm::distance2(getPositionFunction(*item), position);
            if (d > std::numeric_limits<float>::epsilon() && d <= maxDistance) neighbours.push_back(item);
          }
        }
      }
    }
    
    return neighbours;
  };
  
  void insert(std::shared_ptr<T> obj) { items.push_back(obj); }
  
  void remove(int index) { items.erase(items.begin() + index); }
  void remove(std::shared_ptr<T> obj) { items.erase(remove(items.begin(), items.end(), obj), items.end()); }
  
  
  int count() const { return items.size(); }
  
  glm::ivec3 getBinCount() const { return binCount; }
  glm::vec3  getBinSize() const { return binSize; }
  
  
protected:
  
  bool isValidBin(int index) const { return index < bins.size(); }
  bool isValidBin (int x, int y) const { return x >= 0 && x < binCount.x && y >= 0 && y < binCount.y; }
  bool isValidBin (const glm::ivec2 & indices) const { return isValidBin(indices.x, indices.y); }
  
  std::vector<T*> getBin(const glm::vec2 & position) { return bins[getBinIndexFromPosition(position)]; };
  
  int getBinIndexFromPosition(const glm::vec2 & pos) const { return to1D((glm::ivec2) getBinIndicesFromPosition(pos)); }
  glm::ivec2 getBinIndicesFromPosition(const glm::vec2 & pos) const { return glm::ivec2(indexX(pos.x), indexY(pos.y)); }
  
  int indexX(float x) const { return floor((x - boundsPosition.x) / binSize.x); }
  int indexY(float y) const { return floor((y - boundsPosition.y) / binSize.y); }
  
  int to1D(int x, int y) const { return x + y * binCount.x; }
  int to1D(const glm::ivec2 & indices) const { return to1D(indices.x, indices.y); }
  
protected:
  glm::vec2 boundsPosition;
  glm::vec2 boundsSize;
  
  glm::vec2 binSize;
  glm::ivec2 binCount;
  
  std::function<glm::vec2(const T&)> getPositionFunction;
  
  std::vector<std::shared_ptr<T>> items;
  std::vector<std::vector<std::shared_ptr<T>> > bins;
};

class ProximityRenderer {
public:
  template<typename T>
  static void draw(const Proximity2D<T> & proximity)
  {
    ofRectangle bounds = ofRectangle(proximity.boundsPosition, proximity.boundsPosition + proximity.boundsSize);
    ofPushMatrix();
    {
      ofxCortex::core::utils::Debug::drawGrid(bounds, proximity.binCount.x, proximity.binCount.y);
    }
    ofPopMatrix();
  }
  
protected:
  ProximityRenderer() = default;
};

}}}

