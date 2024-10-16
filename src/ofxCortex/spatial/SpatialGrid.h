#pragma once

namespace ofxCortex { namespace core { namespace spatial {

struct SpatialGridItem {
  SpatialGridItem(int index, float x, float y, float radius) : index(index), position(x, y), radius(radius) {};
  
  glm::vec2 position;
  int index;
  float radius;
};

class SpatialCell {
public:
  glm::ivec2 offset;
  glm::vec2 origin;
  float dimension;
  
  std::vector<SpatialGridItem> contents;
  
  SpatialCell(int x, int y, float dimension)
  : offset(x, y), origin(x * dimension, y * dimension), dimension(dimension)
  {}
  
  bool containsPoint(const glm::vec2 & pos) { return pos.x >= origin.x && pos.x < origin.x + dimension && pos.y >= origin.y && pos.y < origin.y + dimension; }
  bool containsPoint(float x, float y) { return containsPoint(glm::vec2(x, y)); }
  
  
  bool intersectsCell(const glm::vec2 & pos, float radius) { return SpatialCell::intersectsBoxCircle(origin.x, origin.x + dimension, origin.y, origin.y + dimension, pos.x, pos.y, radius); }
  bool intersectsCell(float x, float y, float radius) { return intersectsCell(glm::vec2(x, y), radius); }
  
  
  bool intersectsChild(const glm::vec2 & pos, float radius)
  {
    for (auto & element : contents)
    {
      if (glm::distance2(pos, element.position) < (radius * radius)) return true;
    }
    
    return false;
  }
  
  bool intersectsChild(float x, float y, float radius) { return intersectsChild(glm::vec2(x, y), radius); }
  
  
  std::vector<SpatialGridItem> getIntersections(const glm::vec2 & position, float radius)
  {
    std::vector<SpatialGridItem> intersections;
    
    for (auto & element : contents)
    {
      if (glm::distance2(position, element.position) < (radius * radius)) intersections.push_back(element);
    }
    
    return intersections;
  }
  
private:
  static bool intersectsBoxCircle(float boxMinX, float boxMaxX, float boxMinY, float boxMaxY, float circleX, float circleY, float circleRadius)
  {
    float closestX = CLAMP(circleX, boxMinX, boxMaxX);
    float closestY = CLAMP(circleY, boxMinY, boxMaxY);
    
    float distanceX = (circleX - closestX);
    float distanceY = (circleY - closestY);
    
    float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
    
    return (distanceSquared < (circleRadius * circleRadius));
  }
};

template<typename T>
class SpatialGrid2D {
public:
  float width;
  float height;
  float cellLength;
  float cellsPerX, cellsPerY;
  std::vector<SpatialCell> gridCells;
  std::vector<T> gridItems;
  
  SpatialGrid2D(float width, float height, float minRadius, float maxRadius)
  : width(width), height(height), cellLength((minRadius + maxRadius) * 0.5 / 1.414213)
  {
    cellsPerX = ceil(width / cellLength);
    cellsPerY = ceil(height / cellLength);
    
    int totalCells = cellsPerX * cellsPerY;
    gridCells.reserve(totalCells);
    gridItems.reserve(totalCells);
    
    for (int y = 0; y < cellsPerY; y++)
    {
      for (int x = 0; x < cellsPerX; x++)
      {
        gridCells.push_back(SpatialCell(x, y, cellLength));
      }
    }
  }
  
  bool add(const T & item, float x, float y, float radius)
  {
    int index = getIndex(x, y);
    
    if (index == -1) return false;
    
    this->add(item, index, x, y, radius);
    
    return true;
  }
  
  bool add(const T & item, int cellIndex, float x, float y, float radius)
  {
    int cellRadius = ceil(radius / cellLength);
    
    int index = gridItems.size();
    gridItems.push_back(item);
    
    SpatialGridItem spatialGridItem(index, x, y, radius);
    
    for (int iy = -cellRadius; iy <= cellRadius; ++iy)
    {
      for (int ix = -cellRadius; ix <= cellRadius; ++ix)
      {
        int neighbor = cellIndex + ix + (iy * cellsPerX);
        if (neighbor < 0 || neighbor >= gridCells.size()) continue;
        
        if (gridCells[neighbor].intersectsCell(x, y, radius)) {
          gridCells[neighbor].contents.push_back(spatialGridItem);
          return true;
        }
      }
    }
    
    return false;
  }
  
  bool addIfOpen(const T & item, float x, float y, float radius)
  {
    int index = getIndex(x, y);
    
    if (!isOpen(index, x, y, radius)) return false;
    
    add(item, index, x, y, radius);
    
    return true;
  }
  
  void remove(const T & item)
  {
    int index = 0;
    
    for (; index < gridItems.size(); ++index)
    {
      if (gridItems[index] == item) { gridItems.erase(gridItems.begin() + index); break; }
    }
    
    for (int i = 0; i < gridCells.size(); ++i)
    {
      auto & cell = gridCells[i];
      
      for (int j = (cell.contents.size() - 1); j >= 0; --j)
      {
        if (cell.contents[j].index == index) cell.contents.erase(cell.contents.begin() + j);
      }
    }
  }
  
  void clear()
  {
    gridItems.clear();
    
    for (auto & cell : gridCells) cell.contents.clear();
  }
  
  bool isOpen(int centerIndex, float x, float y, float radius)
  {
    if (centerIndex == -1) return false;
    
    int cellRadius = ceil(radius / cellLength);
    
    for (int iy = -cellRadius; iy <= cellRadius; ++iy)
    {
      for (int ix = -cellRadius; ix <= cellRadius; ++ix)
      {
        int neighbor = centerIndex + ix + (iy * cellsPerX);
        
        if (neighbor < 0 || (neighbor >= gridCells.size())) continue;
        
        if (gridCells[neighbor].intersectsCell(x, y, radius) &&
            gridCells[neighbor].intersectsChild(x, y, radius))
        {
          return false;
        }
      }
    }
    
    return true;
  }
  
  int getIndex(float x, float y)
  {
    if (x < 0.0f || x > width || y < 0.0f || y > height) return -1;
    
    int dx = (int)(x / cellLength);
    int dy = (int)(y / cellLength);
    
    return dx + dy * cellsPerX;
  }
  
  std::vector<T> getAt(float x, float y, float radius)
  {
    std::vector<T> result;
    int cellIndex = getIndex(x, y);
    
    if (cellIndex != -1)
    {
      std::vector<SpatialGridItem> items = gridCells[cellIndex].getIntersections(x, y, radius);
      
      for (auto & item : items) result.push_back(gridItems[item.index]);
    }
    
    return result;
  }
  
  
  
protected:
  
  
};

}}}
