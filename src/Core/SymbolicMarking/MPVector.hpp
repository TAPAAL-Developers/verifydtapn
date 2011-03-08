#ifndef MPVECTOR_HPP_
#define MPVECTOR_HPP_

#include <stdlib.h>
#include <string.h>
#include <exception>
#include <stdint.h>
#include <limits.h>

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

class InvalidDimException: public std::exception {
	virtual const char* what() const throw () {
		return "Invalid dimensions";
	}
} invdimex;

const int NegInf = INT_MIN;

class MPVector {
	int n;
	int* val;

public:
	MPVector();
	MPVector(int dim);
	MPVector(const MPVector& mpv);
	~MPVector();

	const bool operator==(const MPVector& mpv);
	const bool operator!=(const MPVector& mpv);
	MPVector& operator+=(const MPVector& mpv);
	const MPVector operator+(const MPVector& mpv);
	MPVector Max(const MPVector& mpv);

	inline int Get(const int idx) const;
	inline void Set(const int idx, const int v);
};

#endif /* MPVECTOR_HPP_ */
