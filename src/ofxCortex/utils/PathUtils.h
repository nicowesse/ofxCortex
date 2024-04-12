#pragma once

#include "ofPath.h"
#include "ofRectangle.h"

namespace ofxCortex { namespace core { namespace utils {

class Path {
public:
  static ofRectangle getPathBoundingBox(const ofPath & path) {
    ofRectangle rect;
    for (int i = 0; i < path.getOutline().size(); i++) {
      ofRectangle b = path.getOutline().at(i).getBoundingBox();
      if (i == 0) rect = b;
      else rect.growToInclude(b);
    }
    return rect;
  }
  
  static std::vector<glm::vec3> pathIntersections(const ofPath & path, const glm::vec3 & start, const glm::vec3 & end)
  {
    std::vector<glm::vec3> output;
    
    for (auto & line : path.getOutline())
    {
      for (int i = 0; i < line.size(); i++)
      {
        glm::vec3 segmentStart = line[i];
        glm::vec3 segmentEnd = line[(i + 1) % line.size()];
        
        glm::vec3 intersection;
        
        if (ofLineSegmentIntersection(start, end, segmentStart, segmentEnd, intersection)) output.push_back(intersection);
      }
    }
    
    return output;
  }
  
  static std::vector<std::pair<ofRectangle, glm::ivec2>> getSlicedPath(const ofPath & path, float spacing)
  {
    ofRectangle bb = getPathBoundingBox(path);
    bb.setFromCenter(bb.getCenter(), bb.getWidth() - 0, bb.getHeight() - 0);
    
    int rows = bb.height / spacing;
    
    std::vector<std::pair<ofRectangle, glm::ivec2>> output;
    
    for (int row = 0; row < rows; row++)
    {
      glm::vec3 start = glm::vec3(bb.getLeft(), bb.y + row * spacing, 0);
      glm::vec3 end = glm::vec3(bb.getRight(), bb.y + row * spacing, 0);
      
      auto rowIntersections = pathIntersections(path, start, end);
      rowIntersections.insert(rowIntersections.begin(), start);
      rowIntersections.insert(rowIntersections.end(), end);
      
      ofSort(rowIntersections, [](const glm::vec3 & a, const glm::vec3 & b) { return a.x < b.x; });
      
      for (int i = 0; i < rowIntersections.size() - 1; i++)
      {
        const glm::vec3 & current = rowIntersections[i];
        const glm::vec3 & next = rowIntersections[i + 1];
        
        float width = glm::distance(current, next);
        
        ofRectangle slice { current.x, current.y, width, spacing };
        output.push_back(std::make_pair( slice, glm::ivec2(i, row) ));
      }
    }
    
    return output;
  }
};

}}}
