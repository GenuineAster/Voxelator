#ifndef MAP_LOADER
#define MAP_LOADER

#include <string>
#include <vector>
#include <memory>

namespace MC {
	struct Location {
		uint32_t offset;
		uint32_t  size;
	};
	struct LocationTable {
		Location table[1024];
	};
	struct TimestampTable {
		uint32_t times[1024];
	};
	struct Chunk {
		std::vector<uint8_t> blocks;
		bool loaded;
	};
}

class MapLoader
{
public:
	MC::LocationTable locations;
	MC::TimestampTable times;
	MC::Chunk chunks[32*32];
	void load(std::string filename);
	MapLoader();
	~MapLoader();
};

#endif