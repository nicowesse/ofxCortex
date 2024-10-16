#pragma once

#include "ofPolyline.h"
#include "ofRectangle.h"

namespace ofxCortex::core::utils {

class PolylineUtils {
private:
  static float getSegDistSq(const glm::vec2 & p,
                     const glm::vec2 & a,
                     const glm::vec2 & b) {
    auto x = a.x;
    auto y = a.y;
    auto dx = b.x - x;
    auto dy = b.y - y;
    
    if (dx != 0 || dy != 0) {
      
      auto t = ((p.x - x) * dx + (p.y - y) * dy) / (dx * dx + dy * dy);
      
      if (t > 1) {
        x = b.x;
        y = b.y;
        
      } else if (t > 0) {
        x += dx * t;
        y += dy * t;
      }
    }
    
    dx = p.x - x;
    dy = p.y - y;
    
    return dx * dx + dy * dy;
  }

  // signed distance from point to polygon outline (negative if point is outside)
  static auto pointToPolygonDist(const glm::vec2& point, const ofPolyline & polygon) {
    bool inside = false;
    auto minDistSq = std::numeric_limits<float>::infinity();
    
  //  for (const auto& ring : polygon) {
      for (std::size_t i = 0, len = polygon.size(), j = len - 1; i < len; j = i++) {
        const auto& a = polygon[i];
        const auto& b = polygon[j];
        
        if ((a.y > point.y) != (b.y > point.y) &&
            (point.x < (b.x - a.x) * (point.y - a.y) / (b.y - a.y) + a.x)) inside = !inside;
        
        minDistSq = std::min(minDistSq, getSegDistSq(point, a, b));
      }
  //  }
    
    return (inside ? 1 : -1) * std::sqrt(minDistSq);
  }

  struct Cell {
    Cell(const glm::vec2& c_, float h_, const ofPolyline & polygon)
    : c(c_),
    h(h_),
    d(pointToPolygonDist(c, polygon)),
    max(d + h * std::sqrt(2))
    {}
    
    glm::vec2 c; // cell center
    float h; // half the cell size
    float d; // distance from cell center to polygon
    float max; // max distance to polygon within a cell
  };

  // get polygon centroid
  static Cell getCentroidCell(const ofPolyline & polygon) {
    float area = 0;
    glm::vec2 c { 0, 0 };
    const auto& ring = polygon;
    
    for (std::size_t i = 0, len = ring.size(), j = len - 1; i < len; j = i++) {
      const glm::vec2& a = ring[i];
      const glm::vec2& b = ring[j];
      auto f = a.x * b.y - b.x * a.y;
      c.x += (a.x + b.x) * f;
      c.y += (a.y + b.y) * f;
      area += f * 3;
    }
    
    return Cell(area == 0 ? ring[0] : c / area, 0, polygon);
  }

public:
  static glm::vec2 getVisualCentroid(const ofPolyline & polygon, float precision = 1, bool debug = false) {
    
    // find the bounding box of the outer ring
    //    const geometry::box<T> envelope = geometry::envelope(polygon.at(0));
    const ofRectangle & envelope = polygon.getBoundingBox();
    
    const glm::vec2 size {
      envelope.getMax().x - envelope.getMin().x,
      envelope.getMax().y - envelope.getMin().y
    };
    
    const float cellSize = std::min(size.x, size.y);
    float h = cellSize / 2;
    
    // a priority queue of cells in order of their "potential" (max distance to polygon)
    auto compareMax = [] (const Cell& a, const Cell& b) {
      return a.max < b.max;
    };
    using Queue = std::priority_queue<Cell, std::vector<Cell>, decltype(compareMax)>;
    Queue cellQueue(compareMax);
    
    if (cellSize == 0) { return (glm::vec2)envelope.getMin(); }
    
    // cover polygon with initial cells
    for (float x = envelope.getMin().x; x < envelope.getMax().x; x += cellSize) {
      for (float y = envelope.getMin().y; y < envelope.getMax().y; y += cellSize) {
        cellQueue.push(Cell({x + h, y + h}, h, polygon));
      }
    }
    
    // take centroid as the first best guess
    auto bestCell = getCentroidCell(polygon);
    
    // second guess: bounding box centroid
    Cell bboxCell(envelope.getMin() + size / 2.0, 0, polygon);
    if (bboxCell.d > bestCell.d) {
      bestCell = bboxCell;
    }
    
    auto numProbes = cellQueue.size();
    while (!cellQueue.empty()) {
      // pick the most promising cell from the queue
      auto cell = cellQueue.top();
      cellQueue.pop();
      
      // update the best cell if we found a better one
      if (cell.d > bestCell.d) {
        bestCell = cell;
        if (debug) std::cout << "found best " << ::round(1e4 * cell.d) / 1e4 << " after " << numProbes << " probes" << std::endl;
      }
      
      // do not drill down further if there's no chance of a better solution
      if (cell.max - bestCell.d <= precision) continue;
      
      // split the cell into four cells
      h = cell.h / 2;
      cellQueue.push(Cell({cell.c.x - h, cell.c.y - h}, h, polygon));
      cellQueue.push(Cell({cell.c.x + h, cell.c.y - h}, h, polygon));
      cellQueue.push(Cell({cell.c.x - h, cell.c.y + h}, h, polygon));
      cellQueue.push(Cell({cell.c.x + h, cell.c.y + h}, h, polygon));
      numProbes += 4;
    }
    
    if (debug) {
      std::cout << "num probes: " << numProbes << std::endl;
      std::cout << "best distance: " << bestCell.d << std::endl;
    }
    
    return bestCell.c;
  }
};

}
