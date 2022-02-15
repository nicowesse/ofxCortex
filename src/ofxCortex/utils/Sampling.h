#pragma once

#include "ofMathConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include <glm/vec2.hpp>

namespace ofxCortex { namespace utils {

struct PoissonDisc {
  static std::vector<glm::vec2> sample(float radius, ofRectangle bounds, int numSamplesBeforeRejection = 30)
  {
    float cellSize = radius / sqrt(2.0);
    
    std::vector<std::vector<int>> grid(ceil(bounds.width / cellSize), std::vector<int>(ceil(bounds.height / cellSize), -1));
    std::vector<glm::vec2> points;
    std::vector<glm::vec2> spawnPoints;
    
    static auto isValid = [&](const glm::vec2 & candidate) -> bool {
      if (candidate.x >= 0.0 && candidate.x <= bounds.width && candidate.y >= 0.0 && candidate.y <= bounds.height)
      {
        int cellX = (int)(candidate.x / cellSize);
        int cellY = (int)(candidate.y / cellSize);
        
        int searchStartX = MAX(0, cellX - 2);
        int searchEndX = MIN(cellX + 2, grid.size() - 1);
        int searchStartY = MAX(0, cellY - 2);
        int searchEndY = MIN(cellX + 2, grid[0].size() - 1);
        
        for (int x = searchStartX; x < searchEndX; x++)
        {
          for (int y = searchStartY; y < searchEndY; y++)
          {
            int pointIndex = grid[x][y];
            if (pointIndex != -1)
            {
              float dst = glm::length2(candidate - points[pointIndex]);
              if (dst < radius * radius) return false;
            }
          }
        }
        
        return true;
      }
    };
    
    spawnPoints.push_back(glm::vec2(bounds.width, bounds.height) / 2.0f);
    while (spawnPoints.size() > 0)
    {
      int spawnIndex = floor(ofRandom(0, spawnPoints.size()));
      glm::vec2 spawnCenter = spawnPoints[spawnIndex];
      bool candidateAccepted = false;
      
      for (int i = 0; i < numSamplesBeforeRejection; i++)
      {
        float a = ofRandom(TWO_PI);
        glm::vec2 dir = glm::vec2(cos(a), sin(a));
        glm::vec2 candidate = spawnCenter + dir * ofRandom(radius, radius * 2.0);
        
        if (isValid(candidate))
        {
          points.push_back(candidate);
          spawnPoints.push_back(candidate);
          grid[(int)(candidate.x / cellSize)][(int)(candidate.y / cellSize)] = points.size();
          candidateAccepted = true;
          break;
        }
      }
      
      if (!candidateAccepted) spawnPoints.erase(spawnPoints.begin() + spawnIndex);
    }
    
    return points;
  }
};

}}
