#ifndef CONST_VEC
#define CONST_VEC

template<typename T>
class const_vec {
public:
	T x,y,z;
	template<typename U>
	constexpr operator const_vec<U>(){return static_cast<const_vec<U>>(*this);}
	constexpr const_vec(T _x, T _y, T _z=T()):x(_x),y(_y),z(_z){}
};

#endif