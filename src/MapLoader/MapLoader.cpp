#include <MapLoader/MapLoader.hpp>

#include <fstream>
#include <iostream>
#include <zlib.h>
#include <NBTParser/NBTParser.hpp>

template<typename T>
T invert_endian(T a) {
	T result = (a&0x000000FF)<<24
	         | (a&0x0000FF00)<< 8
	         | (a&0x00FF0000)>> 8
	         | (a&0xFF000000)>>24;
	return result;
}

void MapLoader::load(std::string filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::in);
	if(!file.is_open())
		return;
	// Parse Location Table
	for(int loc=0;loc<1024;++loc) {
		int tmp;
		file.read(reinterpret_cast<char*>(&tmp), 4);
		locations.table[loc].size = (tmp>>24)*4096;//(tmp&0xFF)*4096;
		locations.table[loc].offset = (invert_endian(tmp&0xFFFFFF)>>8)*4096;
	}
	// Parse Timestamp Table
	for(int t=0;t<1024;++t) {
		int tmp;
		file.read(reinterpret_cast<char*>(&tmp), 4);
		tmp = invert_endian(tmp);
		times.times[t] = tmp;
	}

	unsigned int max=0;
	for(int i=0;i<1024;++i) {
		if(times.times[i] > max)
			max = times.times[i];
	}
	std::cout<<"Largest timestamp was "<<max<<std::endl;

	max = 0;
	for(int i=0;i<1024;++i) {
		if((locations.table[i].offset)+(locations.table[i].size) > max)
			max = (locations.table[i].offset)+(locations.table[i].size);
	}
	std::cout<<"Filesize was "<<max<<std::endl;

	for(int i=0;i<1024;++i) {
		if(locations.table[i].size == 0)
			continue;
		chunks[i].data = new uint8_t[locations.table[i].size];
		file.seekg(locations.table[i].offset);
		file.read(reinterpret_cast<char*>(chunks[i].data), locations.table[i].size);
		uint32_t length = invert_endian(*reinterpret_cast<uint32_t*>(chunks[i].data));
		uint8_t  compression = chunks[i].data[4];
		if(compression == 2) {
			uint8_t *uncompressed_data = new uint8_t[length];
			uLongf len = length;
			uncompress(
				uncompressed_data,
				&len,
				const_cast<const uint8_t*>(&chunks[i].data[5]),
				locations.table[i].size-4
			);
			if(i==0) {
				for(int pos=0;pos<len;++pos) {
					
				}
			}
			auto tag = parse_nbt(uncompressed_data, len, 0);
		}
	}

	file.close();
}

MapLoader::MapLoader() {

}

MapLoader::~MapLoader() {

}
