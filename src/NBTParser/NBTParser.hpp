#ifndef NBT_PARSER
#define NBT_PARSER

#include <string>
#include <vector>
#include <memory>

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
		End();
	};

	class Byte : public Tag {
	public:
		int8_t data;
		const Type type = Type::BYTE;
		Byte();
	};

	class Short : public Tag {
	public:
		int16_t data;
		const Type type = Type::SHORT;
		Short();
	};

	class Int : public Tag {
	public:
		int32_t data;
		const Type type = Type::INT;
		Int();
	};

	class Long : public Tag {
	public:
		int64_t data;
		const Type type = Type::LONG;
		Long();
	};

	class Float : public Tag {
	public:
		float data;
		const Type type = Type::FLOAT;
		Float();
	};

	class Double : public Tag {
	public:
		double data;
		const Type type = Type::DOUBLE;
		Double();
	};

	class Byte_Array : public Tag {
	public:
		std::vector<Tags::Byte> data;
		const Type type = Type::BYTE_ARRAY;
		Byte_Array();
	};

	class String : public Tag {
	public:
		std::string data;
		const Type type = Type::STRING;
		String();
	};

	class List : public Tag {
	public:
		std::vector<std::shared_ptr<Tags::Tag>> data;
		const Type type = Type::LIST;
		Type list_type;
		List();
	};

	class Compound : public Tag {
	public:
		std::vector<std::shared_ptr<Tags::Tag>> data;
		const Type type = Type::COMPOUND;
		std::shared_ptr<Tags::Tag> operator[](std::string val);
		Compound();
	};

	class Int_Array : public Tag {
	public:
		std::vector<Tags::Int> data;
		const Type type = Type::INT_ARRAY;
		Int_Array();
	};
}

std::shared_ptr<Tags::Compound> parse_nbt(uint8_t *data, uint32_t len, uint32_t cursor);

#endif