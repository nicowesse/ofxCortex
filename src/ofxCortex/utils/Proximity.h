#pragma once

#include <vector>
#include "ofVectorMath.h"

namespace ofxCortex { namespace core { namespace utils {
    template <class T>
    class ProximityManager
    {
      
    public:
      
      ProximityManager(glm.:ivec3 bins, glm::vec3 size = glm::vec3(), glm::vec3 position = glm::vec3())
      : _binCount(numBinsX), _size(size), _position(position)
      {
        _bins.resize(_binCount.x * _binCount.y * _binCount.znum);
      };
      
      void update()
      {
        for (int i = 0; i < _bins.size(); i++) { _bins[i].clear(); }
        
        _binSize = _size / _binCount;
        
        int num = _binCount.x * _binCount.y * _binCount.z;
        
        for (int i = 0; i < _objs.size(); i++)
        {
          int xBin, yBin, zBin;
          getBinIndices(getObjectPosition(_objs[i]), xBin, yBin, zBin);
          
          int binIndex = getBinIndex(xBin, yBin, zBin);
          
          if (isValidBin(xBin, yBin, zBin))
          {
            _bins[binIndex].push_back(_objs[i]);
          }
        }
      };
      
      vector<T*> getNeighbours(glm::vec3 position, float radius)
      {
        int binX, binY, binZ;
        
        int index = getBinIndex(position, binX, binY, binZ);
        
        int minXBin = getXBinIndex(position.x - radius);
        int maxXBin = getXBinIndex(position.x + radius);
        int minYBin = getYBinIndex(position.y - radius);
        int maxYBin = getYBinIndex(position.y + radius);
        int minZBin = getZBinIndex(position.z - radius);
        int maxZBin = getZBinIndex(position.z + radius);
        
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
                vector<T *> objs = _bins[getBinIndex(x, y, z)];
                
                for (int i = 0; i < objs.size(); i++)
                {
                  float distance = glm::distance2(getObjectPosition(objs[i]), position);
                  
                  if (distance <= maxDistance)
                  {
                    neighbours.push_back(objs[i]);
                  }
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
      
      glm::ivec3 getNumBins() const { return _binCount; }
      glm::vec3 getBinSize() const { return _binSize; }
      
      
    protected:
      
      bool isValidBin (int x, int y, int z)
      {
        return x > -1 && x < _binCount.x && y > -1 && y < _binCount.y && (_binCount.z < 1 || (z > -1 && z < _binCount.z));
      }
      
      int getBinIndices(glm::vec3 pos, int & x, int & y, int & z) const
      {
        x = getXBinIndex(pos.x);
        y = getYBinIndex(pos.y);
        z = (_numBinsZ > 0) ? getZBinIndex(pos.z) : 0;
        
        return 3Dto2D(x, y, z);
      }
      
      int indexX(float x) { return floor((x - _position.x) / _binSize.x); }
      int indexY(float y) { return floor((y - _position.y) / _binSize.y); }
      int indexZ(float z) { return floor((z - _position.z) / _binSize.z); }
      
      int 3Dto2D (int x, int y, int z)
      {
        int binIndex = (y * _binCount.x) + x;
        binIndex += z * (_binCount.x * _binCunt.y);
        
        return binIndex;
      }
      
      vector<T*> getBin(glm::vec3 position)
      {
        int x, y, z;
        int index = getBinIndices(position, x, y, z);
        
        return _bins[index];
      };
      
      
      
      virtual glm::vec3 & getObjectPosition(T * obj)
      {
        return obj->position;
      };
      
    protected:
      ofRectangle _bounds;
      glm::vec3 _binSize;
      glm::ivec3 _binCount;
      
      vector<T*> _objs;
      vector<vector<T*> > _bins;
    };
  
}}}

