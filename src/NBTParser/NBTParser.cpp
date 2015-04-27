#include <NBTParser/NBTParser.hpp>

#include <iostream>
#include <iomanip>
#include <exception>

std::shared_ptr<Tags::End> parse_end(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Byte> parse_byte(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Short> parse_short(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Int> parse_int(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Long> parse_long(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Float> parse_float(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Double> parse_double(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Byte_Array> parse_byte_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::String> parse_string(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth, bool is_name=false);
std::shared_ptr<Tags::List> parse_list(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Compound> parse_compound(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);
std::shared_ptr<Tags::Int_Array> parse_int_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth);

std::shared_ptr<Tags::Compound> parse_nbt(uint8_t *data, uint32_t len, uint32_t cursor) {
	return parse_compound(data, len, cursor, true, true, 0);
}


std::shared_ptr<Tags::Byte> parse_byte(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Byte>();

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::BYTE) {
			return std::make_shared<Tags::Byte>();
		}
	}

	std::shared_ptr<Tags::Byte> tag = std::make_shared<Tags::Byte>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}

	tag->data = data[++c];


	return tag;
}


std::shared_ptr<Tags::Short> parse_short(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Short>(Tags::Short());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::SHORT) {
			return std::make_shared<Tags::Short>(Tags::Short());
		}
	}

	std::shared_ptr<Tags::Short> tag = std::make_shared<Tags::Short>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}

	tag->data = 0;
	tag->data |= static_cast<int16_t>(data[++c])<<8;
	tag->data |= data[++c];


	return tag;
}

std::shared_ptr<Tags::Int> parse_int(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Int>(Tags::Int());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::INT) {
			return std::make_shared<Tags::Int>(Tags::Int());
		}
	}

	std::shared_ptr<Tags::Int> tag = std::make_shared<Tags::Int>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}

	tag->data = 0;
	tag->data |= static_cast<int32_t>(data[++c])<<24;
	tag->data |= static_cast<int32_t>(data[++c])<<16;
	tag->data |= static_cast<int32_t>(data[++c])<<8;
	tag->data |= data[++c];


	return tag;
}

std::shared_ptr<Tags::Long> parse_long(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Long>(Tags::Long());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::LONG) {
			return std::make_shared<Tags::Long>(Tags::Long());
		}
	}

	std::shared_ptr<Tags::Long> tag = std::make_shared<Tags::Long>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}

	tag->data = 0;
	tag->data |= static_cast<int64_t>(data[++c])<<56;
	tag->data |= static_cast<int64_t>(data[++c])<<48;
	tag->data |= static_cast<int64_t>(data[++c])<<40;
	tag->data |= static_cast<int64_t>(data[++c])<<32;
	tag->data |= static_cast<int64_t>(data[++c])<<24;
	tag->data |= static_cast<int64_t>(data[++c])<<16;
	tag->data |= static_cast<int64_t>(data[++c])<<8;
	tag->data |= data[++c];


	return tag;
}

std::shared_ptr<Tags::Float> parse_float(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Float>(Tags::Float());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::FLOAT) {
			return std::make_shared<Tags::Float>(Tags::Float());
		}
	}

	std::shared_ptr<Tags::Float> tag = std::make_shared<Tags::Float>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}

	uint32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	tag->data = *reinterpret_cast<float*>(&tmp);


	return tag;
}

std::shared_ptr<Tags::Double> parse_double(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Double>(Tags::Double());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::DOUBLE) {
			return std::make_shared<Tags::Double>(Tags::Double());
		}
	}

	std::shared_ptr<Tags::Double> tag = std::make_shared<Tags::Double>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
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

	tag->data = *reinterpret_cast<double*>(&tmp);


	return tag;
}

std::shared_ptr<Tags::Byte_Array> parse_byte_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Byte_Array>(Tags::Byte_Array());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::BYTE_ARRAY) {
			return std::make_shared<Tags::Byte_Array>(Tags::Byte_Array());
		}
	}

	std::shared_ptr<Tags::Byte_Array> tag = std::make_shared<Tags::Byte_Array>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}


	int32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	tag->data.resize(tmp);

	for(uint32_t i=0;i<tmp;++i) {
		tag->data[i].data = data[++c];
	}

	return tag;
}

std::shared_ptr<Tags::String> parse_string(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth, bool is_name) {
	if(cursor >= len)
		return std::make_shared<Tags::String>(Tags::String());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::STRING) {
			return std::make_shared<Tags::String>(Tags::String());
		}
	}

	std::shared_ptr<Tags::String> tag = std::make_shared<Tags::String>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}



	uint16_t str_size = 0;
		str_size = str_size | (static_cast<int16_t>(data[++c])<<8);
		str_size = str_size | data[++c];

	tag->data.resize(str_size);

	for(uint16_t i=0;i<str_size;++i) {
		tag->data[i] = data[++c];
	}

	return tag;
}

std::shared_ptr<Tags::List> parse_list(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::List>(Tags::List());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::LIST) {
			return std::make_shared<Tags::List>(Tags::List());
		}
	}

	std::shared_ptr<Tags::List> tag = std::make_shared<Tags::List>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}


	uint8_t type = data[++c];

	tag->list_type = static_cast<Tags::Type>(type);

	int32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	tag->data.resize(tmp);

	for(uint32_t i=0;i<tmp;++i) {
		switch(type) {
			case Tags::Type::BYTE: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_byte(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::SHORT: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_short(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::INT: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_int(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::LONG: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_long(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::FLOAT: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_float(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::DOUBLE: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_double(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::BYTE_ARRAY: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_byte_array(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::STRING: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_string(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::INT_ARRAY: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_int_array(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::LIST: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_list(data, len, c, false, false, depth+1));
			} break;
			case Tags::Type::COMPOUND: {
				tag->data[i] = std::static_pointer_cast<Tags::Tag>(parse_compound(data, len, c, false, false, depth+1));
			} break;
			default: {
			} break;
		}
	}


	return tag;
}

std::shared_ptr<Tags::Compound> parse_compound(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Compound>(Tags::Compound());

	uint32_t &c = cursor;

	if(has_tag) {
		if(data[c] != Tags::Type::COMPOUND) {
			return std::make_shared<Tags::Compound>(Tags::Compound());
		}
	}

	std::shared_ptr<Tags::Compound> tag = std::make_shared<Tags::Compound>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}


	while(data[++c] != Tags::END) {
		switch(data[c]) {
			case Tags::Type::BYTE: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_byte(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::SHORT: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_short(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::INT: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_int(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::LONG: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_long(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::FLOAT: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_float(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::DOUBLE: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_double(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::BYTE_ARRAY: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_byte_array(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::STRING: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_string(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::LIST: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_list(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::COMPOUND: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_compound(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::INT_ARRAY: {
				tag->data.push_back(std::static_pointer_cast<Tags::Tag>(parse_int_array(data, len, c, true, true, depth+1)));
			} break;
			case Tags::Type::END: {
				return tag;
			} break;
			default: {
			} break;
		}
	}


	return tag;
}

std::shared_ptr<Tags::Int_Array> parse_int_array(uint8_t *data, uint32_t len, uint32_t &cursor, bool has_tag, bool has_name, int depth) {
	if(cursor >= len)
		return std::make_shared<Tags::Int_Array>(Tags::Int_Array());

	uint32_t &c = cursor;
	if(has_tag) {
		if(data[c] != Tags::Type::INT_ARRAY) {
			return std::make_shared<Tags::Int_Array>(Tags::Int_Array());
		}
	}

	std::shared_ptr<Tags::Int_Array> tag = std::make_shared<Tags::Int_Array>();

	if(has_name) {
		tag->name = parse_string(data, len, c, false, false, depth+1, true)->data;
	}


	int32_t tmp = 0;
	tmp |= static_cast<uint32_t>(data[++c])<<24;
	tmp |= static_cast<uint32_t>(data[++c])<<16;
	tmp |= static_cast<uint32_t>(data[++c])<<8;
	tmp |= data[++c];

	tag->data.resize(tmp);

	for(uint32_t i = 0;i<tmp;++i) {
		tag->data[i].data = parse_int(data, len, c, false, false, depth+1)->data;
	}

	return tag;
}

Tags::Tag::Tag() : type(Tags::Type::TAG) {
	;
}
Tags::End::End() : type(Tags::Type::END) {
	;
}
Tags::Byte::Byte() : type(Tags::Type::BYTE) {
	;
}
Tags::Short::Short() : type(Tags::Type::SHORT) {
	;
}
Tags::Int::Int() : type(Tags::Type::INT) {
	;
}
Tags::Long::Long() : type(Tags::Type::LONG) {
	;
}
Tags::Float::Float() : type(Tags::Type::FLOAT) {
	;
}
Tags::Double::Double() : type(Tags::Type::DOUBLE) {
	;
}
Tags::Byte_Array::Byte_Array() : type(Tags::Type::BYTE_ARRAY) {
	;
}
Tags::String::String() : type(Tags::Type::STRING) {
	;
}
Tags::List::List() : type(Tags::Type::LIST) {
	;
}
Tags::Compound::Compound() : type(Tags::Type::COMPOUND) {
	;
}
Tags::Int_Array::Int_Array() : type(Tags::Type::INT_ARRAY) {
	;
}

std::shared_ptr<Tags::Tag> Tags::Compound::operator[](std::string val) {
	for(auto &t : data) {
		if(t->name == val)
			return t;
	}
	throw -1;
}
