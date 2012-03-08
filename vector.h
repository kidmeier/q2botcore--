#ifndef __vector_H__
#define __vector_H__

#include <assert.h>
#include <cmath>
#include <cstdio>

#include "q2bot.h"

typedef float real;
class vector
{
	real _v[3];
public:
	vector(real x = 0.0, real y = 0.0, real z = 0.0)  {
		_v[0] = x; _v[1] = y; _v[2] = z;
	}
	
	vector(const real* v) {
		_v[0] = v[0];
		_v[1] = v[1];
		_v[2] = v[2];
	}
	~vector() {}

	real length() const {
		return std::sqrt( _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] );
	}
	
	real length2() const {
		return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2];
	}
	
	vector normal() const {
		return vector( _v[0]/length(), _v[1]/length(), _v[2]/length() );
	}
	
	real  normalize() {
	
		real len = length();
		_v[0] /= len;
		_v[1] /= len;
		_v[2] /= len;
		return len;
	}
	
	static vector to_angles(vector v) {
		v.to_angles();
		return v;
	}
	
	void to_angles() {    
		real forward;
		real yaw, pitch;
		
		forward = std::sqrt( _v[0]*_v[0] + _v[1]*_v[1] );       
		pitch = std::atan2(_v[2], forward);
		yaw = std::atan2(_v[1], _v[0]);
		
		_v[0] = pitch;
		_v[1] = yaw;
		_v[2] = 0.0;				
	}

	//      vector to_vector()
	//      {

	// accessors
	::vec3_t* vec3_t() { return &_v; }
	real&  operator[] (int i) {
		assert(i >= 0 && i <= 2);
		return _v[i];
	}
	
	real  x() const { return _v[0]; }
	real  y() const { return _v[1]; }
	real  z() const { return _v[2]; }

	// basic vector math
	vector& operator+= (const vector& a) {
		_v[0] += a._v[0];
		_v[1] += a._v[1];
		_v[2] += a._v[2];
		return *this;
	}
	
	vector& operator-= (const vector& a) {
		_v[0] -= a._v[0];
		_v[1] -= a._v[1];
		_v[2] -= a._v[2];
		return *this;
	}
	
	vector& operator*= (const real r) {
		_v[0] *= r;
		_v[1] *= r;
		_v[2] *= r;
		return *this;
	}
	
	friend vector  operator+ (const vector& a, const vector& b);
	friend vector  operator- (const vector& a, const vector& b);
	friend real    operator* (const vector& a, const vector& b);
	friend vector  operator* (const vector& a, const real r);
	friend vector  operator% (const vector& a, const vector& b); 
};

inline vector operator+ (const vector& a, const vector& b) {
	return vector( a._v[0]+b._v[0], a._v[1]+b._v[1], a._v[2]+b._v[2] );
}

inline vector operator- (const vector& a, const vector& b) {
	return vector( a._v[0]-b._v[0], a._v[1]-b._v[1], a._v[2]-b._v[2] );
}

//inline vector operator- (const vec3_t a, const vec3_t b) {
//	return vector(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
//}

inline real operator* ( const vector& a, const vector& b ) {
	return a._v[0]*b._v[0] + a._v[1]*b._v[1] + a._v[2]*b._v[2];
}

//inline real	operator* ( const vec3_t a, const vec3_t b ) {
//	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];

inline vector  operator* ( const vector& a, const real r ) {
	return vector( a._v[0]*r, a._v[1]*r, a._v[2]*r );
}

inline vector operator% ( const vector& a, const vector& b ) {
	return vector(  a._v[1]*b._v[2] - a._v[2]*b._v[1],
					a._v[0]*b._v[2] - a._v[2]*b._v[0],
					a._v[0]*b._v[1] - a._v[1]*b._v[0]
					);
}

#endif

