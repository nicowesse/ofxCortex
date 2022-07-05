#pragma once

#include <vector>
#include "ofVectorMath.h"

namespace ofxCortex { namespace core { namespace utils {

    template <class T>
    class Proximity
    {
    public:
      Proximity() = default;
      
//      Proximity(glm::ivec3 bins, glm::vec3 position = glm::vec3(), glm::vec3 size = glm::vec3())
//      : _binCount(bins), _position(position), _size(size)
//      {
//
//      };
      
      void setup(glm::ivec3 bins, glm::vec3 position = glm::vec3(), glm::vec3 size = glm::vec3())
      {
        _binCount = bins;
        _position = position;
        _size = size;
        
        _bins.resize(_binCount.x * _binCount.y * _binCount.z);
      }
      
      void update()
      {
        for (auto & bin : _bins) { bin.clear(); }
        
        _binSize = _size / (glm::vec3) _binCount;
        
        for (auto & obj : _objs)
        {
          glm::ivec3 indices = getBinIndices(getObjectPosition(obj));
          
          if (isValidBin(indices))
          {
            _bins[getBinIndex(indices)].push_back(obj);
          }
        }
      };
      
      vector<T*> getNeighbours(glm::vec3 position, float radius)
      {
        
        int minXBin = indexX(position.x - radius);
        int maxXBin = indexX(position.x + radius);
        int minYBin = indexY(position.y - radius);
        int maxYBin = indexY(position.y + radius);
        int minZBin = indexZ(position.z - radius);
        int maxZBin = indexZ(position.z + radius);
        
        if (maxXBin < minXBin) swap(maxXBin, minXBin);
        if (maxYBin < minYBin) swap(maxYBin, minYBin);
        if (maxZBin < minZBin) swap(maxZBin, minZBin);
        
        vector<T *> neighbours;
        
        float maxDistance = radius * radius;
        
        for (int x = minXBin; x <= maxXBin; x++)
        {
          for (int y = minYBin; y <= maxYBin; y++)
          {
            for (int z = minZBin; z <= maxZBin; z++)
            {
              if (isValidBin(x, y, z))
              {
                vector<T *> objs = _bins[to1D(x, y, z)];
                
                for (int i = 0; i < objs.size(); i++)
                {
                  float distance = glm::distance2(getObjectPosition(objs[i]), position);
                  
                  if (distance <= maxDistance) neighbours.push_back(objs[i]);
                }
              }
            }
          }
        }
        
        return neighbours;
      };
      
      void insert(T *obj) { _objs.push_back(obj); }
      
      void remove(int index) { _objs.erase(_objs.begin() + index); }
      void remove(T * obj) { _objs.erase(remove(_objs.begin(), _objs.end(), obj), _objs.end()); }
      
      
      int count() const { return _objs.size(); }
      
      glm::ivec3 getBinCount() const { return _binCount; }
      glm::vec3  getBinSize() const { return _binSize; }
      
      
    protected:
      
      bool isValidBin (int x, int y, int z)
      {
        return x >= 0 && x < _binCount.x && y >= 0 && y < _binCount.y && (_binCount.z < 1 || (z >= 0 && z < _binCount.z));
      }
      
      bool isValidBin (glm::ivec3 indices) { return isValidBin(indices.x, indices.y, indices.z); }
      
      vector<T*> getBin(glm::vec3 position)
      {
        return _bins[getBinIndex(position)];
      };
      
      int getBinIndex(glm::vec3 pos) const
      {
        return to1D((glm::ivec3) getBinIndices(pos));
      }
      
      glm::ivec3 getBinIndices(glm::vec3 pos) const
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
      
      virtual glm::vec3 getObjectPosition(T * obj)
      {
        return *obj;
      };
      
    protected:
      glm::vec3 _position;
      glm::vec3 _size;
      
      glm::vec3 _binSize;
      glm::ivec3 _binCount;
      
      vector<T*> _objs;
      vector<vector<T*> > _bins;
    };
  
}}}

