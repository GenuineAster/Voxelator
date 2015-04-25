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
	struct Region {
		MC::LocationTable locations;
		MC::TimestampTable times;
		Chunk chunks[1024];
	};
}

class MapLoader
{
public:
	std::vector<std::vector<MC::Region>> regions;
	void load(std::string filename, int offset_x, int offset_y);
	MapLoader();
	~MapLoader();
};

#endif