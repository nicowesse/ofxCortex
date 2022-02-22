#pragma once

#include "ofMathConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include <glm/vec2.hpp>

namespace ofxCortex { namespace utils {

struct PoissonDisc {
public:
  PoissonDisc() {};
  PoissonDisc(ofRectangle bounds) : bounds(bounds) {};
  void setBounds(ofRectangle bounds) { this->bounds = bounds; }
  
  std::vector<glm::vec2> sample(float radius, int numSamplesBeforeRejection = 30)
  {
    return PoissonDisc::sample(radius, this->bounds, numSamplesBeforeRejection);
  }
  
  static std::vector<glm::vec2> sample(float radius, ofRectangle bounds, int numSamplesBeforeRejection = 30)
  {
    radius = MAX(1.0, radius);
    
    float cellSize = radius / sqrt(2.0);
    
    int columns = ceil(bounds.width / cellSize);
    int rows = ceil(bounds.height / cellSize);
    
    std::vector<int> grid(columns * rows, -1);
    std::vector<glm::vec2> points;
    std::vector<glm::vec2> spawnPoints;
    
    static auto isValid = [&, bounds](const glm::vec2 & candidate) -> bool {
      if (candidate.x >= bounds.x && candidate.x <= bounds.x + bounds.width && candidate.y >= bounds.y && candidate.y <= bounds.y + bounds.height)
      {
        int cellX = (int)(candidate.x / cellSize);
        int cellY = (int)(candidate.y / cellSize);
        
        int searchStartX =  MAX(cellX - 2, 0);
        int searchEndX =    MIN(cellX + 2, columns);
        int searchStartY =  MAX(cellY - 2, 0);
        int searchEndY =    MIN(cellY + 2, rows);
        
        for (int x = searchStartX; x < searchEndX; x++)
        {
          for (int y = searchStartY; y < searchEndY; y++)
          {
            int pointIndex = grid[x + y * columns];
            if (pointIndex != -1)
            {
              float dst = glm::length2(candidate - points[pointIndex]);
              if (dst < radius * radius) return false;
            }
          }
        }
        
        return true;
      }
      
      return false;
    };
    
    spawnPoints.push_back(glm::vec2(bounds.width, bounds.height) / 2.0f);
    
    int tries = 0;
    while (spawnPoints.size() > 0 && tries < 100000)
    {
      int spawnIndex = (int) (ofRandom(0, spawnPoints.size()));
      glm::vec2 spawnCenter = spawnPoints[spawnIndex];
      bool candidateAccepted = false;
      
      for (int i = 0; i < numSamplesBeforeRejection; i++)
      {
        float a = ofRandom(TWO_PI);
        glm::vec2 dir = { cos(a), sin(a) };
        glm::vec2 candidate = spawnCenter + dir * ofRandom(radius, radius * 2.0);
        
        if (isValid(candidate))
        {
          points.push_back(candidate);
          spawnPoints.push_back(candidate);
          int col = (int)(candidate.x / cellSize);
          int row = (int)(candidate.y / cellSize);
          grid[col + row * columns] = points.size() - 1;
          candidateAccepted = true;
          break;
        }
      }
      
      if (!candidateAccepted) spawnPoints.erase(spawnPoints.begin() + spawnIndex);
      
      tries++;
    }
    
    return points;
  }
  
private:
  ofRectangle bounds;
};

}}
