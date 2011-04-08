#ifndef MPVECTOR_HPP_
#define MPVECTOR_HPP_

#include <stdlib.h>
#include <string.h>
#include <exception>
#include <stdint.h>
#include <limits.h>
#include <iostream>
#include <vector>

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
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
const int FirstClock = 1;

class MPVector {
	int n;
	std::vector<int> val;

public:
	MPVector() : n(0) { };
	explicit MPVector(int dim, int defVal = 0);

	const bool operator==(const MPVector& mpv) const;
	const bool operator!=(const MPVector& mpv) const;
	int operator+(const MPVector& mpv) const;
	MPVector& operator+=(const int v);
	const MPVector operator+(const int v) const;
	const bool operator<(const MPVector& mpv) const;
	const bool operator<=(const MPVector& mpv) const;
	const bool operator>=(const MPVector& mpv) const;
	const bool operator>(const MPVector& mpv) const;
	MPVector Max(const MPVector& mpv) const;

	int Get(const int idx) const;
	void Set(const int idx, const int v);
	int GetDim() const;

	void AddDim(int value = 0);
	void RemoveDim(int index);
	void Swap(int dim1, int dim2);

	friend std::ostream &operator<<(std::ostream &out, const MPVector &value) {
		out << "(";
		for (int i = 0; i < value.n; i++) {
			if (i!=0) out << ",";
			if (value.val[i] == NegInf)
				out << "-INF";
			else
				out << value.val[i];
		}
		out << ")";
		return out;
	}
};

MPVector Max(const MPVector& lhs, const MPVector& rhs);
const MPVector operator+(const int v, const MPVector& mpv);

#endif /* MPVECTOR_HPP_ */
