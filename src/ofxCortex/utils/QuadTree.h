#pragma once

#include "ofMain.h"

constexpr size_t MAX_QUADTREE_DEPTH = 8;

template<typename T>
class QuadTree {
public:
  QuadTree(size_t currentDepth = 0)
  : depth(currentDepth)
  {}
  
  void setup(const ofRectangle & viewport = ofRectangle(0, 0, 100, 100), const std::function<ofRectangle(const T&)> & getItemArea = [](const T & item){ return item.rectangle; }, const std::function<glm::vec2(const T&)> & getItemPosition = [](const T & item){ return item.position; })
  {
    this->getArea = getItemArea;
    this->getPosition = getItemPosition;
    this->resize(viewport);
  }
  
  void resize(const ofRectangle & rect)
  {
    this->clear();
    this->rect = rect;
    
    const glm::vec2 pos = this->rect.getPosition();
    glm::vec2 quadSize = glm::vec2(rect.width, rect.height) / 2.0;
    
    quads = {
      ofRectangle(pos, quadSize),
      ofRectangle(pos + glm::vec2(quadSize.x, 0), quadSize),
      ofRectangle(pos + glm::vec2(0, quadSize.y), quadSize),
      ofRectangle(pos + quadSize, quadSize)
    };
  }
  
  void clear()
  {
    items.clear();
    
    for (int i = 0; i < 4; i++)
    {
      if (branches[i]) branches[i]->clear();
      branches[i].reset();
    }
  }
  
  void insert(const T& item)
  {
    for (int i = 0; i < 4; i++)
    {
      if (quads[i].inside(getArea(item)))
      {
        if (depth + 1 > MAX_QUADTREE_DEPTH)
        {
          if (!branches[i])
          {
            branches[i] = std::make_shared<QuadTree<T>>(depth + 1);
            branches[i]->setup(quads[i], getArea, getPosition);
          }
          
          branches[i]->insert(item);
          return;
        }
      }
    }
    
    items.push_back(item);
  }
  
  std::list<T> searchArea(const ofRectangle & area) const
  {
    std::list<T> output;
    searchArea(area, output);
    return output;
  }
  
  void searchArea(const ofRectangle & area, std::list<T> & searchItems) const
  {
    for (const auto & p : items)
    {
      if (area.intersects(getArea(p))) { searchItems.push_back(p); }
    }
    
    for (int i = 0; i < 4; i++)
    {
      if (branches[i])
      {
        if (area.inside(quads[i])) branches[i]->getItems(searchItems);
        else if (quads[i].intersects(area)) branches[i]->searchArea(area, searchItems);
      }
    }
  }
  
  std::list<T> searchRadius(const glm::vec2 & position, float radius) const
  {
    std::list<T> output;
    searchRadius(position, radius, output);
    return output;
  }
  
  void searchRadius(const glm::vec2 & position, float radius, std::list<T> & searchItems) const
  {
    const float maxRadius = radius * radius;
    for (const auto & p : items)
    {
      if (glm::distance2(position, getPosition(p)) < maxRadius) { searchItems.push_back(p); }
    }
    
    for (int i = 0; i < 4; i++)
    {
      if (branches[i])
      {
        if (quads[i].inside(position)) branches[i]->searchRadius(position, radius, searchItems);
      }
    }
  }
  
  void getItems(std::list<T> & searchItems)
  {
    for (const auto & p : items) searchItems.push_back(p);
    
    for (int i = 0; i < 4; i++)
    {
      if (branches[i]) branches[i]->getItems(searchItems);
    }
  }
  
  size_t size()
  {
    size_t count = items.size();
    for (int i = 0; i < 4; i++)
    {
      if (branches[i]) count += branches[i]->size();
    }
    return count;
  }
  
  const ofRectangle & area() const { return rect; }
  
protected:
  size_t depth = 0;
  
  ofRectangle rect;
  std::array<ofRectangle, 4> quads;
  std::array<std::shared_ptr<QuadTree<T>>, 4> branches;
  std::vector<T> items;
  
  std::function<ofRectangle(const T&)> getArea;
  std::function<glm::vec2(const T&)> getPosition;
};
