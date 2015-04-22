#include <NBTParser/NBTParser.hpp>

#include <iostream>
#include <iomanip>


Tags::End parse_end(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Byte parse_byte(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Short parse_short(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Int parse_int(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Long parse_long(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Float parse_float(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Double parse_double(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Byte_Array parse_byte_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::String parse_string(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::List parse_list(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Compound parse_compound(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);
Tags::Int_Array parse_int_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent);

Tags::Compound parse_nbt(uint8_t *data, uint32_t len, uint32_t cursor) {
	return parse_compound(data, len, cursor, true, true, 0);
}



Tags::Byte parse_byte(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing byte"<<std::endl;

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::BYTE) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Byte();
		}
	}

	Tags::Byte tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	tag.data = data[++c];

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<+tag.data<<std::endl;

	return tag;
}


Tags::Short parse_short(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing short"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::SHORT) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Short();
		}
	}

	Tags::Short tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	tag.data = 0;
	tag.data |= static_cast<int16_t>(data[++c])<<8;
	tag.data |= data[++c];

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::Int parse_int(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing int"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::INT) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Int();
		}
	}

	Tags::Int tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	tag.data = 0;
	tag.data |= static_cast<int32_t>(data[++c])<<24;
	tag.data |= static_cast<int32_t>(data[++c])<<16;
	tag.data |= static_cast<int32_t>(data[++c])<<8;
	tag.data |= data[++c];

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::Long parse_long(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing long"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::LONG) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Long();
		}
	}

	Tags::Long tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	tag.data = 0;
	tag.data |= static_cast<int64_t>(data[++c])<<56;
	tag.data |= static_cast<int64_t>(data[++c])<<48;
	tag.data |= static_cast<int64_t>(data[++c])<<40;
	tag.data |= static_cast<int64_t>(data[++c])<<32;
	tag.data |= static_cast<int64_t>(data[++c])<<24;
	tag.data |= static_cast<int64_t>(data[++c])<<16;
	tag.data |= static_cast<int64_t>(data[++c])<<8;
	tag.data |= data[++c];

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::Float parse_float(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing float"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::FLOAT) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Float();
		}
	}

	Tags::Float tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	uint32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	tag.data = *reinterpret_cast<float*>(&tmp);

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::Double parse_double(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing double"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::DOUBLE) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Double();
		}
	}

	Tags::Double tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	uint64_t tmp;
	tmp |= static_cast<uint64_t>(data[++c])<<56;
	tmp |= static_cast<uint64_t>(data[++c])<<48;
	tmp |= static_cast<uint64_t>(data[++c])<<40;
	tmp |= static_cast<uint64_t>(data[++c])<<32;
	tmp |= static_cast<uint64_t>(data[++c])<<24;
	tmp |= static_cast<uint64_t>(data[++c])<<16;
	tmp |= static_cast<uint64_t>(data[++c])<<8;
	tmp |= data[++c];

	tag.data = *reinterpret_cast<double*>(&tmp);

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::Byte_Array parse_byte_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing byte array"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::BYTE_ARRAY) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Byte_Array();
		}
	}

	Tags::Byte_Array tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}
	}

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<" {";

	uint32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	for(int i=0;i<tmp;++i) {
		Tags::Byte d;
		d.data = data[++c];
		tag.data.push_back(d);
		if(i%8==0) {
			std::cout<<std::endl;
			for(int n=0;n<indent+1;++n)std::cout<<"\t";
		}
		std::cout<<std::setw(3)<<+data[c]<<", ";
	}
	std::cout<<std::endl;
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"}"<<std::endl;
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"elements: "<<tmp<<std::endl;

	return tag;
}

Tags::String parse_string(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing string"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::STRING) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::String();
		}
	}

	Tags::String tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	uint16_t str_size = 0;
		str_size = str_size | (static_cast<int16_t>(data[++c])<<8);
		str_size = str_size | data[++c];

	for(int i=0;i<str_size;++i) {
		tag.data += data[++c];
	}

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tag.data<<std::endl;

	return tag;
}

Tags::List parse_list(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing list"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::LIST) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::List();
		}
	}

	Tags::List tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	uint32_t type = data[++c];

	tag.list_type = static_cast<Tags::Type>(type);

	size_t sz;

		uint32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	for(int i=0;i<tmp;++i) {
		switch(type) {
			case Tags::Type::BYTE: {
				auto t = parse_byte(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::SHORT: {
				auto t = parse_short(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::INT: {
				auto t = parse_int(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::LONG: {
				auto t = parse_long(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::FLOAT: {
				auto t = parse_float(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::DOUBLE: {
				auto t = parse_double(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::BYTE_ARRAY: {
				auto t = parse_byte_array(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::STRING: {
				auto t = parse_string(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::INT_ARRAY: {
				auto t = parse_int_array(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::LIST: {
				auto t = parse_list(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			case Tags::Type::COMPOUND: {
				auto t = parse_compound(data, len, c, false, false, indent+1);
				tag.data.push_back(new auto(t));
			} break;
			default: {
				for(int n=0;n<indent;++n)std::cout<<"\t";
				std::wcout<<L"HELP ME NOW AAAAAH";
			} break;
		}
	}

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tmp<<" elements long "<<std::endl;

	return tag;
}

Tags::Compound parse_compound(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing compound"<<std::endl;
	uint32_t &c = cursor;

	if(has_tag) {
		if(data[c] != Tags::Type::COMPOUND) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Compound();
		}
	}

	Tags::Compound tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	while(data[++c] != Tags::END) {
		switch(data[c]) {
			case Tags::Type::BYTE: {
				tag.data.push_back(new auto(parse_byte(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::SHORT: {
				tag.data.push_back(new auto(parse_short(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::INT: {
				tag.data.push_back(new auto(parse_int(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::LONG: {
				tag.data.push_back(new auto(parse_long(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::FLOAT: {
				tag.data.push_back(new auto(parse_float(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::DOUBLE: {
				tag.data.push_back(new auto(parse_double(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::BYTE_ARRAY: {
				tag.data.push_back(new auto(parse_byte_array(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::STRING: {
				tag.data.push_back(new auto(parse_string(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::LIST: {
				tag.data.push_back(new auto(parse_list(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::COMPOUND: {
				tag.data.push_back(new auto(parse_compound(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::INT_ARRAY: {
				tag.data.push_back(new auto(parse_int_array(data, len, c, true, true, indent+1)));
			} break;
			case Tags::Type::END: {
				break;
			} break;
			default: {
				for(int n=0;n<indent;++n)std::cout<<"\t";
				std::cout<<"Something is broken?!"<<std::endl;
				// ++c;
			} break;
		}
	}

	return tag;
}

Tags::Int_Array parse_int_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int indent) {
	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<"Parsing int array"<<std::endl;
	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::INT_ARRAY) {
			for(int n=0;n<indent;++n)std::cout<<"\t";
			std::cout<<"WRONG TYPE!!!!"<<std::endl;
			return Tags::Int_Array();
		}
	}

	Tags::Int_Array tag;

	if(has_name) {
		int16_t name_size = 0;
			name_size = name_size | (static_cast<int16_t>(data[++c])<<8);
			name_size = name_size | data[++c];
		uint8_t name_data[name_size];
		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<"Name length is : "<<name_size<<std::endl;

		for(int i=0;i<name_size;++i) {
			name_data[i] = data[++c];
			tag.name += data[c];
		}

		for(int n=0;n<indent;++n)std::cout<<"\t";
		std::cout<<tag.name<<std::endl;
	}

	uint32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	for(int n=0;n<indent;++n)std::cout<<"\t";
	std::cout<<tag.name<<":"<<tmp<<" bytes long "<<std::endl;

	// c+= tmp * sizeof(int);

	return tag;
}

Tags::Tag::Tag() : type(Tags::Type::TAG) {
	;
}

Tags::Compound::Compound() {
	;
}
Tags::Compound::~Compound() {
	;
}

Tags::List::List() {
	;
}
Tags::List::~List() {
	;
}