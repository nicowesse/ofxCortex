#pragma once

#include "ofEvents.h"

namespace ofxCortex { namespace core {
  static ofEventListeners listeners;
}}

#include "ofxCortex/core/UUID.h"
#include "ofxCortex/core/Serialization.h"

#include "ofxCortex/utils/Helpers.h"
#include "ofxCortex/utils/ShapingUtils.h"
#include "ofxCortex/utils/TimingUtils.h"
#include "ofxCortex/utils/PathUtils.h"
#include "ofxCortex/utils/ParameterUtils.h"
#include "ofxCortex/utils/AudioUtils.h"
#include "ofxCortex/utils/VectorUtils.h"
#include "ofxCortex/utils/3DUtils.h"
#include "ofxCortex/utils/GraphicUtils.h"
#include "ofxCortex/utils/DebugUtils.h"

#include "ofxCortex/spatial/Proximity.h"
#include "ofxCortex/spatial/QuadTree.h"
#include "ofxCortex/spatial/SpatialGrid.h"

#include "ofxCortex/graphics/Line.h"
#include "ofxCortex/graphics/Typography.h"

#include "ofxCortex/generators/Waveform.h"
#include "ofxCortex/generators/Noise.h"
#include "ofxCortex/generators/Sampling.h"

#include "ofxCortex/types/AllTypes.h"
