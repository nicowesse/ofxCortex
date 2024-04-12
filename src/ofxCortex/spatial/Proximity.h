#pragma once

#include <vector>
#include "ofVectorMath.h"
#include "ofxCortex/utils/GraphicUtils.h"

namespace ofxCortex { namespace core { namespace utils {

template <class T>
class Proximity
{
public:
  friend class ProximityRenderer;
  
  Proximity() = default;
  
  void setup(std::function<glm::vec3(const T&)> getPositionFunc, glm::ivec3 bins = glm::ivec3(10, 10, 1), glm::vec3 position = glm::vec3(), glm::vec3 size = glm::vec3(ofGetWidth(), ofGetHeight(), 1))
  {
    _binCount = bins;
    _position = position;
    _size = size;
    _binSize = _size / (glm::vec3) _binCount;
    
    _bins.resize(_binCount.x * _binCount.y * MIN(1, _binCount.z)); //(bins.z > 0 ? _binCount.z : 1));
    _getPositionFunction = getPositionFunc;
  }
  
  void update()
  {
    for (auto & bin : _bins) { bin.clear(); }
    
    _binSize = _size / (glm::vec3) _binCount;
    
    for (auto & obj : _objs)
    {
      int index = getBinIndexFromPosition(_getPositionFunction(*obj));
      
      if (isValidBin(index)) { _bins[index].push_back(obj); }
    }
  };
  
  std::vector<std::shared_ptr<T>> getNearby(glm::vec3 position, float radius) const
  {
    if (!_getPositionFunction)
    {
      ofLogWarning() << "No position function!";
      return std::vector<std::shared_ptr<T>>();
    }
    
    int minXBin = indexX(position.x - radius);
    int maxXBin = indexX(position.x + radius);
    int minYBin = indexY(position.y - radius);
    int maxYBin = indexY(position.y + radius);
    int minZBin = indexZ(position.z - radius);
    int maxZBin = indexZ(position.z + radius);
    
    if (maxXBin < minXBin) std::swap(maxXBin, minXBin);
    if (maxYBin < minYBin) std::swap(maxYBin, minYBin);
    if (maxZBin < minZBin) std::swap(maxZBin, minZBin);
    
    std::vector<std::shared_ptr<T>> neighbours;
    
    float maxDistance = radius * radius;
    
    for (int x = minXBin; x <= maxXBin; x++)
    {
      for (int y = minYBin; y <= maxYBin; y++)
      {
        for (int z = minZBin; z <= maxZBin; z++)
        {
          if (isValidBin(x, y, z))
          {
            auto objs = _bins[to1D(x, y, z)];
            
            for (int i = 0; i < objs.size(); i++)
            {
              float distance = glm::distance2(_getPositionFunction(*objs[i]), position);
              
              if (distance <= maxDistance) neighbours.push_back(objs[i]);
            }
          }
        }
      }
    }
    
    return neighbours;
  };
  
  void insert(std::shared_ptr<T> obj) {
    //        _objs.push_back(obj);
    
    int index = getBinIndexFromPosition(_getPositionFunction(*obj));
    if (isValidBin(index)) { _bins[index].push_back(obj); }
  }
  
  void remove(int index) { _objs.erase(_objs.begin() + index); }
  void remove(std::shared_ptr<T> obj) { _objs.erase(remove(_objs.begin(), _objs.end(), obj), _objs.end()); }
  
  
  int count() const { return _objs.size(); }
  
  glm::ivec3 getBinCount() const { return _binCount; }
  glm::vec3  getBinSize() const { return _binSize; }
  
  
protected:
  
  bool isValidBin(int index) const { return index < _bins.size(); }
  
  bool isValidBin (int x, int y, int z) const
  {
    return x >= 0 && x < _binCount.x && y >= 0 && y < _binCount.y && (_binCount.z < 1 || (z >= 0 && z < _binCount.z));
  }
  
  bool isValidBin (glm::ivec3 indices) const { return isValidBin(indices.x, indices.y, indices.z); }
  
  std::vector<T*> getBin(glm::vec3 position)
  {
    return _bins[getBinIndexFromPosition(position)];
  };
  
  int getBinIndexFromPosition(glm::vec3 pos) const
  {
    return to1D((glm::ivec3) getBinIndicesFromPosition(pos));
  }
  
  glm::ivec3 getBinIndicesFromPosition(glm::vec3 pos) const
  {
    return glm::ivec3(indexX(pos.x), indexY(pos.y), (_binCount.z > 0) ? indexZ(pos.z) : 0);
  }
  
  int indexX(float x) const { return floor((x - _position.x) / _binSize.x); }
  int indexY(float y) const { return floor((y - _position.y) / _binSize.y); }
  int indexZ(float z) const { return floor((z - _position.z) / _binSize.z); }
  
  int to1D(int x, int y, int z) const
  {
    int binIndex = (y * _binCount.x) + x;
    binIndex += z * (_binCount.x * _binCount.y);
    
    return binIndex;
  }
  
  int to1D(glm::ivec3 indices) const { return to1D(indices.x, indices.y, indices.z); }
  
protected:
  glm::vec3 _position;
  glm::vec3 _size;
  
  glm::vec3 _binSize;
  glm::ivec3 _binCount;
  
  std::function<glm::vec3(const T&)> _getPositionFunction;
  
  std::vector<std::shared_ptr<T>> _objs;
  std::vector<std::vector<std::shared_ptr<T>> > _bins;
};

class ProximityRenderer {
public:
  template<typename T>
  static void draw(const Proximity<T> & proximity)
  {
    if (proximity._binCount.z > 1) // 3D
    {
      
    }
    else
    {
      ofRectangle bounds = ofRectangle(proximity._position, proximity._position + proximity._size);
      ofPushMatrix();
      {
        ofxCortex::core::utils::Debug::drawGrid(bounds, proximity._binCount.x, proximity._binCount.y);
      }
      ofPopMatrix();
    }
  }
  
protected:
  ProximityRenderer() = default;
};

}}}

