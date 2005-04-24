//
//  A 4d (homogenous) vector (C)+(W) 2005 Thorsten Jordan
//

#ifndef VECTOR4_H
#define VECTOR4_H

#ifdef WIN32
#undef min
#undef max
#endif

#include "vector3.h"

template<class D>
class vector4t
{
	public:
	D x, y, z, w;

	vector4t() : x(0), y(0), z(0), w(0) {}
	vector4t(const D& x_, const D& y_, const D& z_, const D& w_) : x(x_), y(y_), z(z_), w(w_) {}
	// not all of the following operations are sensible for homogenous vectors.
	vector4t<D> normal(void) const { D len = D(1.0)/length(); return vector4t(x * len, y * len, z * len, w * len); }
	void normalize(void) { D len = D(1.0)/length(); x *= len; y *= len; z *= len; w *= len; }
	vector4t<D> operator* (const D &scalar) const { return vector4t(x * scalar, y * scalar, z * scalar, w * scalar); }
	vector4t<D> operator+ (const vector4t<D>& other) const { return vector4t(x + other.x, y + other.y, z + other.z, w + other.w); }
	vector4t<D> operator- (const vector4t<D>& other) const { return vector4t(x - other.x, y - other.y, z - other.z, w - other.w); }
	vector4t<D> operator- (void) const { return vector4t(-x, -y, -z, -w); };
	vector4t<D>& operator+= (const vector4t<D>& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
	vector4t<D>& operator-= (const vector4t<D>& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
	bool operator== (const vector4t<D>& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
	D square_length(void) const { return x * x + y * y + z * z + w * w; }
	D length(void) const { return D(sqrt(square_length())); }
	D square_distance(const vector4t<D>& other) const { vector4t<D> n = *this - other; return n.square_length(); }
	D distance(const vector4t<D>& other) const { vector4t<D> n = *this - other; return n.length(); }
	D operator* (const vector4t<D>& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
	vector3t<D> xyz(void) const { return vector3t<D>(x, y, z); }
	vector3t<D> to_real(void) const { return (w == 0) ? vector3t<D>() : vector3t<D>(x/w, y/w, z/w); }
	template<class D2> friend std::ostream& operator<< ( std::ostream& os, const vector4t<D2>& v );
	template<class E> void assign(const vector4t<E>& other) { x = D(other.x); y = D(other.y); z = D(other.z); w = D(other.w); }
};

template<class D2> inline vector4t<D2> operator* (const D2& scalar, const vector4t<D2>& v) { return v * scalar; }

template<class D>
std::ostream& operator<< ( std::ostream& os, const vector4t<D>& v )
{
	os << "x=" << v.x << "; y=" << v.y << "; z=" << v.z << "; w=" << v.w;
	return os;
}

typedef vector4t<double> vector4;
typedef vector4t<float> vector4f;
typedef vector4t<int> vector4i;

#endif