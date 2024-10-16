#pragma once

#include "ofMathConstants.h"
#include "ofUtils.h"
#include "ofMath.h"
#include "ofVectorMath.h"
#include "ofRectangle.h"
#include <glm/vec2.hpp>
#include "ofxCortex/types/Box.h"
#include "ofxCortex/spatial/SpatialGrid.h"

namespace ofxCortex { namespace core { namespace generators {

struct PoissonDisc {
public:
  PoissonDisc() {};
  PoissonDisc(ofRectangle bounds) : bounds(bounds) {};
  void setBounds(ofRectangle bounds) { this->bounds = bounds; }
  
  std::vector<glm::vec2> sample(float radius, int numSamplesBeforeRejection = 30)
  {
    return PoissonDisc::sample(radius, this->bounds, numSamplesBeforeRejection);
  }
  
  static std::vector<glm::vec2> sample(float radius, const ofRectangle & bounds, int numSamplesBeforeRejection = 32)
  {
    return PoissonDisc::sampleFromDensityFunction(radius, radius, bounds, [](const glm::vec2 &) { return 1.0f; }, numSamplesBeforeRejection);
  }
  
  static std::vector<glm::vec2> sampleFromDensityFunction(float minRadius, float maxRadius, const ofRectangle & bounds, const std::function<float(const glm::vec2&)> & radiusFunc, int numSamplesBeforeRejection = 32)
  {
    float cellSize = minRadius / 1.41421f;
    
    const int columns = ceil(bounds.width / cellSize);
    const int rows = ceil(bounds.height / cellSize);
    
    std::vector<int> grid(columns * rows, -1);
    std::vector<size_t> active; active.reserve(columns * rows);
    
    std::vector<glm::vec2> samples;
    
    auto addSample = [&](const glm::vec2 & sample) {
      const size_t sampleIndex = samples.size();
      
      samples.push_back(sample);
      active.push_back(sampleIndex);
      
      int col = (int)(sample.x / cellSize);
      int row = (int)(sample.y / cellSize);
      
      grid[col + row * columns] = sampleIndex;
      
      return true;
    };
    
    auto isValid = [&](const glm::vec2 & candidate, float radius) -> bool {
      if (!bounds.inside(candidate)) return false;
      
      const int offset = 3;
      const float radius2 = radius * radius;
      
      const int cellX = (int)(candidate.x / cellSize);
      const int cellY = (int)(candidate.y / cellSize);
      
      const int searchStartX =  MAX(cellX - offset, 0);
      const int searchEndX =    MIN(cellX + offset, columns);
      const int searchStartY =  MAX(cellY - offset, 0);
      const int searchEndY =    MIN(cellY + offset, rows);
      
      for (int x = searchStartX; x < searchEndX; ++x)
      {
        for (int y = searchStartY; y < searchEndY; ++y)
        {
          int pointIndex = grid[x + y * columns];
          if (pointIndex != -1 && glm::length2(candidate - samples[pointIndex]) < radius2) return false;
        }
      }
      
      return true;
    };
    
    addSample(glm::vec2(bounds.width, bounds.height) / 2.0f);
    
    int tries = 0;
    while (active.size() > 0 && tries < 100000)
    {
      int sampleIndex = floor(ofRandom(active.size()));
      glm::vec2 currentSample = samples[active[sampleIndex]];
      
      bool candidateAccepted = false;
      
      float sampleRadius = ofMap(radiusFunc(currentSample), 0.0, 1.0, minRadius, maxRadius, true);
      
      for (int i = 0; i < numSamplesBeforeRejection; i++)
      {
        float a = ofRandom(TWO_PI);
        glm::vec2 dir = { cos(a), sin(a) };
        glm::vec2 candidate = currentSample + dir * ofRandom(sampleRadius, sampleRadius * 2.0);
        
        if (isValid(candidate, sampleRadius) && addSample(candidate))
        {
          candidateAccepted = true;
          break;
        }
      }
      
      if (!candidateAccepted) active.erase(active.begin() + sampleIndex);
      
      tries++;
    }
    
    for (auto & point : samples) point += glm::vec2(bounds.x, bounds.y);
    
    return samples;
  }
  
  
  static std::vector<glm::vec3> sample3D(float radius, const ofMesh & mesh, int numSamplesBeforeRejection = 30)
  {
    radius = MAX(1.0, radius);
    
    float cellSize = radius / 1.41421f;
    ofxCortex::core::types::Box bounds = ofxCortex::core::utils::getMeshBoundingBox(mesh);
    ofxCortex::core::types::Box originalBounds = bounds;
    bounds.x = 0;
    bounds.y = 0;
    bounds.z = 0;
    
    int columns = ceil(bounds.width / cellSize);
    int rows = ceil(bounds.height / cellSize);
    int layers = ceil(bounds.depth / cellSize);
    
    std::vector<int> grid(columns * rows * layers, -1);
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> spawnPoints;
    
    auto isValid = [&, bounds](const glm::vec3 & candidate) -> bool {
      if (bounds.inside(candidate))
      {
        int cellX = (int)(candidate.x / cellSize);
        int cellY = (int)(candidate.y / cellSize);
        int cellZ = (int)(candidate.z / cellSize);
        
        int searchStartX =  MAX(cellX - 2, 0);
        int searchEndX =    MIN(cellX + 2, columns);
        int searchStartY =  MAX(cellY - 2, 0);
        int searchEndY =    MIN(cellY + 2, rows);
        int searchStartZ =  MAX(cellZ - 2, 0);
        int searchEndZ =    MIN(cellZ + 2, rows);
        
        for (int x = searchStartX; x < searchEndX; x++)
        {
          for (int y = searchStartY; y < searchEndY; y++)
          {
            for (int z = searchStartZ; z < searchEndZ; z++)
            {
              int gridIndex = x + rows * (y + columns * z);
              int pointIndex = grid[gridIndex];
              if (pointIndex != -1 && pointIndex < points.size())
              {
                float dst = glm::length2(candidate - points[pointIndex]);
                if (dst < radius * radius) return false;
              }
            }
          }
        }
        
        return true;
      }
      
      return false;
    };
    
    spawnPoints.push_back(glm::vec3(bounds.width, bounds.height, bounds.depth) / 2.0f);
    
    int tries = 0;
    while (spawnPoints.size() > 0)
    {
      int spawnIndex = (int) (ofRandom(0, spawnPoints.size() - 1));
      glm::vec3 spawnCenter = spawnPoints[spawnIndex];
      bool candidateAccepted = false;
      
      for (int i = 0; i < numSamplesBeforeRejection; i++)
      {
        glm::vec3 candidate = spawnCenter + ofxCortex::core::utils::Vector::random3D(ofRandom(radius, radius * 2.0));
        
        if (isValid(candidate))
        {
          points.push_back(candidate);
          spawnPoints.push_back(candidate);
          
          int col = (int)(candidate.x / cellSize);
          int row = (int)(candidate.y / cellSize);
          int lay = (int)(candidate.z / cellSize);
          int index = col + rows * (row + columns * lay);
          grid[index] = points.size() - 1;
          candidateAccepted = true;
          break;
        }
      }
      
      if (!candidateAccepted) spawnPoints.erase(spawnPoints.begin() + spawnIndex);
      
      tries++;
    }
    
    for (auto & point : points) point += originalBounds.position;
    
    std::vector<glm::vec3> output;
    std::copy_if(points.begin(), points.end(), back_inserter(output), [&mesh](const glm::vec3 & p) { return ofxCortex::core::utils::isInsideMesh(p, mesh); });
    
    return output;
  }
  
  static std::vector<glm::vec2> insidePolyline(const ofPolyline & source, float radius)
  {
    std::vector<glm::vec2> samples = sample(radius, source.getBoundingBox());
    
    std::vector<glm::vec2> output;
    for (const glm::vec2 & point : samples)
    {
      if (source.inside(point.x, point.y)) output.push_back(point);
    }
    
    return output;
  }
  
  static std::vector<glm::vec2> fromMask(const ofFbo & mask, float radius, float threshold = 0.9f)
  {
    ofRectangle bb(0, 0, mask.getWidth(), mask.getHeight());
    std::vector<glm::vec2> samples = sample(radius, bb);
    
    ofFloatPixels pixels;
    mask.readToPixels(pixels);
    
    std::vector<glm::vec2> output;
    for (const glm::vec2 & point : samples)
    {
      if (pixels.getColor(point.x, point.y).getBrightness() >= threshold) output.push_back(point);
    }
    return output;
  }
  
private:
  ofRectangle bounds;
};

class VariablePoissonDisc {
public:
  
  static std::vector<glm::vec2> sample(const ofRectangle & bounds, float minRadius, float maxRadius, std::function<float(const glm::vec2&)> radiusLookup)
  {
    const int rejectionLimit = 30;
    
    auto spatialGrid = spatial::SpatialGrid2D<int>(bounds.width, bounds.height, minRadius, maxRadius);
    std::vector<int> activeList(spatialGrid.cellsPerX * spatialGrid.cellsPerY);
    std::vector<glm::vec2> samples;
    
    auto addSample = [&](const glm::vec2 & sample) 
    {
      int sampleIndex = samples.size();
      
      samples.push_back(sample);
      activeList.push_back(sampleIndex);
    };
    
    glm::vec2 firstSample { ofRandom(bounds.width), ofRandom(bounds.height) };
    addSample(firstSample);
    
    while (activeList.size() > 0)
    {
      bool sampleFound = false;
      int activeIndex = floor(ofRandom(activeList.size()));
      
      glm::vec2 currentSample = samples[activeList[activeIndex]];
      float radius = radiusLookup(currentSample);
      
      for (int i = 0; i < rejectionLimit; ++i)
      {
        glm::vec2 randomSample = currentSample + ofxCortex::core::utils::Vector::random2D(ofRandom(radius, radius * 2.0));
        
        if (spatialGrid.addIfOpen(samples.size(), randomSample.x, randomSample.y, radius))
        {
          addSample(randomSample);
          sampleFound = true;
          
          break;
        }
      }
      
      if (!sampleFound) activeList.erase(activeList.begin() + activeIndex);
    }
    
    return samples;
  }
  
private:
  
  
};

}}}
