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
};

const int NegInf = INT_MIN;
const int ConeIdx = 0;
const int ZeroIdx = 1;

class MPVector {
	int n;
	int* val;

public:
	MPVector();
	MPVector(int dim, int defVal = 0);
	MPVector(const MPVector& mpv);
	const MPVector& operator=(const MPVector& mpv);

	~MPVector();

	const bool operator==(const MPVector& mpv) const;
	const bool operator!=(const MPVector& mpv) const;
	MPVector& operator+=(const MPVector& mpv);
	const MPVector operator+(const MPVector& mpv) const;
	const bool operator<(const MPVector& mpv) const;
	const bool operator<=(const MPVector& mpv) const;
	const bool operator>=(const MPVector& mpv) const;
	const bool operator>(const MPVector& mpv) const;
	MPVector Max(const MPVector& mpv) const;

	int Get(const int idx) const;
	void Set(const int idx, const int v);
	int GetDim() const;
};

MPVector Max(const MPVector& lhs, const MPVector& rhs);

#endif /* MPVECTOR_HPP_ */
