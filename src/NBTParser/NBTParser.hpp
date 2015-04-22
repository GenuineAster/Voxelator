#ifndef NBT_PARSER
#define NBT_PARSER

#include <string>
#include <vector>

namespace Tags {
	enum Type {
		TAG = -1,
		END = 0,
		BYTE = 1,
		SHORT = 2,
		INT = 3,
		LONG = 4,
		FLOAT = 5,
		DOUBLE = 6,
		BYTE_ARRAY = 7,
		STRING = 8,
		LIST = 9,
		COMPOUND = 10,
		INT_ARRAY = 11
	};
	class Tag {
	public:
		const Type type;
		std::string name;
		Tag();
	};

	class End : public Tag {
	public:
		const Type type = Type::END;
	};

	class Byte : public Tag {
	public:
		int8_t data;
		const Type type = Type::BYTE;
	};

	class Short : public Tag {
	public:
		int16_t data;
		const Type type = Type::SHORT;
	};

	class Int : public Tag {
	public:
		int32_t data;
		const Type type = Type::INT;
	};

	class Long : public Tag {
	public:
		int64_t data;
		const Type type = Type::LONG;
	};

	class Float : public Tag {
	public:
		float data;
		const Type type = Type::FLOAT;
	};

	class Double : public Tag {
	public:
		double data;
		const Type type = Type::DOUBLE;
	};

	class Byte_Array : public Tag {
	public:
		std::vector<Tags::Byte> data;
		const Type type = Type::BYTE_ARRAY;
	};

	class String : public Tag {
	public:
		std::string data;
		const Type type = Type::STRING;
	};

	class List : public Tag {
	public:
		std::vector<Tags::Tag*> data;
		const Type type = Type::LIST;
		Type list_type;
		List();
		~List();
	};

	class Compound : public Tag {
	public:
		std::vector<Tag*> data;
		const Type type = Type::COMPOUND;
		Compound();
		~Compound();
	};

	class Int_Array : public Tag {
	public:
		std::vector<int32_t> data;
		const Type type = Type::INT_ARRAY;
	};
}


// class NBTParser
// {

// };

Tags::Compound parse_nbt(uint8_t *data, uint32_t len, uint32_t cursor);

#endif