#ifndef MPVECTOR_HPP_
#define MPVECTOR_HPP_

#include <stdlib.h>
#include <string.h>
#include <exception>
#include <stdint.h>
#include <limits.h>
#include <iostream>
#include <vector>

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

class InvalidDimException: public std::exception {
	virtual const char* what() const throw () {
		return "Invalid dimensions";
	}
};

const int NegInf = INT_MIN;
const int ConeIdx = 0;
const int FirstClock = 1;

class MPVectorCOW;

class MPVector {
	friend class MPVectorCOW;
	int n;
	int refs;
	std::vector<int> val;

public:
	MPVector() : n(0), refs(1) { };
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

	const MPVector Normalize() const;

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

class MPVectorCOW {
private:
	MPVector* data;
	MPVectorCOW(MPVector* mpv) {
		data = mpv;
	}
public:
	MPVectorCOW() { data = new MPVector(); };
	explicit MPVectorCOW(int dim, int defVal = 0) : data(new MPVector(dim, defVal)) { };
	MPVectorCOW(const MPVectorCOW& other) : data(other.data) {
		if (data)
			++data->refs;
	}

	MPVectorCOW& operator=(const MPVectorCOW& other) {
	   MPVector* old = data;
	   data = other.data;
	   ++data->refs;
	   if (old && --old->refs < 1) delete old;
	   return *this;
	}

	~MPVectorCOW() {
		if (data && --data->refs < 1)
			delete data;
	}

	void CheckCopy() {
		if (data->refs > 1) {
			MPVector* newData = new MPVector(*data);
			--data->refs;
			data = newData;
		}
	}

	const bool operator==(const MPVectorCOW& mpv) const {
		return *data == *mpv.data;
	}

	const bool operator!=(const MPVectorCOW& mpv) const {
		return *data != *mpv.data;
	}

	int operator+(const MPVectorCOW& mpv) const {
		return *data + *mpv.data;
	}

	MPVectorCOW& operator+=(const int v) {
		CheckCopy();
		*data += v;
		return *this;
	}

	const MPVectorCOW operator+(const int v) const {
		return MPVectorCOW(new MPVector((*data)+v));
	}

	const bool operator<(const MPVectorCOW& mpv) const {
		return *data < *mpv.data;
	}

	const bool operator<=(const MPVectorCOW& mpv) const {
		return *data <= *mpv.data;
	}

	const bool operator>=(const MPVectorCOW& mpv) const {
		return *data >= *mpv.data;
	}

	const bool operator>(const MPVectorCOW& mpv) const {
		return *data > *mpv.data;
	}

	MPVectorCOW Max(const MPVectorCOW& mpv) const {
		return MPVectorCOW(new MPVector(data->Max(*mpv.data)));
	}

	int Get(const int idx) const {
		return data->Get(idx);
	}

	void Set(const int idx, const int v) {
		CheckCopy();
		data->Set(idx, v);
	}

	int GetDim() const {
		return data->GetDim();
	}

	void AddDim(int value = 0) {
		CheckCopy();
		data->AddDim(value);
	}

	void RemoveDim(int index) {
		CheckCopy();
		data->RemoveDim(index);
	}

	void Swap(int dim1, int dim2) {
		CheckCopy();
		data->Swap(dim1, dim2);
	}

	const MPVectorCOW Normalize() const {
		return MPVectorCOW(new MPVector(data->Normalize()));
	}

	friend std::ostream &operator<<(std::ostream &out, const MPVectorCOW &v) {
		return out << *v.data;
	}
};

MPVectorCOW Max(const MPVectorCOW& lhs, const MPVectorCOW& rhs);

const MPVectorCOW operator+(const int v, const MPVectorCOW& mpv);

#endif /* MPVECTOR_HPP_ */
