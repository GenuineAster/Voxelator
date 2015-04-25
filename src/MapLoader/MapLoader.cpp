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
		locations.table[loc].size = (tmp>>24)*4096;
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

	std::vector<uint8_t> compressed_data(4*1024*1024);
	std::vector<uint8_t> uncompressed_data(4*1024*1024);

	for(int i=0;i<1024;++i) {
		chunks[i].loaded=false;
		if(locations.table[i].offset == 0)
			continue;
		if(locations.table[i].size == 0)
			continue;
		// chunks[i].data.resize(locations.table[i].size);
		file.seekg(locations.table[i].offset);
		file.read(reinterpret_cast<char*>(compressed_data.data()), locations.table[i].size);

		uint32_t length = invert_endian(*reinterpret_cast<uint32_t*>(compressed_data.data()));
		uint8_t  compression = compressed_data[4];
		if(compression == 2) {
			uLongf len = uncompressed_data.size();
			uncompress(
				uncompressed_data.data(),
				&len,
				const_cast<const uint8_t*>(&(compressed_data[5])),
				length-1
			);
			auto tag = parse_nbt(uncompressed_data.data(), len, 0);

			std::shared_ptr<Tags::Compound> level = std::static_pointer_cast<Tags::Compound>(tag->data[0]);
			std::shared_ptr<Tags::List> sections;


			try {
				sections = std::static_pointer_cast<Tags::List>((*level)["Sections"]);
			} catch(int e) {
				continue;
			}
			
			chunks[i].blocks.resize(16*16*256);
			for(auto &block : chunks[i].blocks)
				block = 0;
			chunks[i].loaded = true;

			for(auto &t : sections->data) {
				std::shared_ptr<Tags::Compound> section = std::static_pointer_cast<Tags::Compound>(t);
				uint32_t y;
				std::vector<Tags::Byte> *blocks;
				try {
					y = static_cast<uint32_t>(std::static_pointer_cast<Tags::Byte>((*section)["Y"])->data);
					blocks = &(std::static_pointer_cast<Tags::Byte_Array>((*section)["Blocks"])->data);
				} catch (int e) {
					continue;
				}

				size_t offset = ((15-y)*16*16*16);

				for(uint32_t _z=0;_z<16;++_z) {
					for(uint32_t _y=0;_y<16;++_y) {
						for(uint32_t _x=0;_x<16;++_x) {
							size_t index_vox = offset+((15-_y)*16*16+_z*16+_x);
							size_t index_mca = _y*16*16+_z*16+_x;
								chunks[i].blocks[index_vox] = (*blocks)[index_mca].data;
						}
					}
				}
			}

		}
		// chunks[i].data.clear();
	}
	uncompressed_data.clear();
	compressed_data.clear();

	file.close();
}

MapLoader::MapLoader() {

}

MapLoader::~MapLoader() {

}
