#include "ofMath.h"
#include "UUID.h"

#include <random>

namespace ofxCortex { namespace core {

static std::random_device _randomDevice;
static std::mt19937_64 _engine(_randomDevice());
static std::uniform_int_distribution<uint64_t> _universalDistribution;

UUID::UUID() : _uuid(_universalDistribution(_engine)) {}

UUID::UUID(uint64_t uuid) : _uuid(uuid) {}

}}
