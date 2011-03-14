#include "MPVector.hpp"

InvalidDimException invdimex;

MPVector::MPVector() {
	n = 0;
	val = NULL;
}

MPVector::MPVector(int dim, int defVal) {
	if (dim == 0) {
		throw invdimex;
	}
	n = dim+1; //Add 1 to leave room for simpler poly/cone conversion
	val = new int[n];
	for (int i = FirstClock; i < n; i++)
		val[i] = defVal;
	val[ConeIdx] = NegInf;
}

MPVector::MPVector(const MPVector& mpv) {
	n = mpv.n;
	if (n == 0) {
		val = NULL;
	} else {
		val = new int[n];
		memcpy(val, mpv.val, sizeof(int) * n);
	}
}

const MPVector & MPVector::operator =(const MPVector & mpv)
{
	if(val) {
		delete[] val;
	}
	n = mpv.n;
	if (n == 0) {
		val = NULL;
	} else {
		val = new int[n];
		memcpy(val, mpv.val, sizeof(int) * n);
	}
	return *this;
}

MPVector::~MPVector() {
	if(val) {
		delete[] val;
	}
}

const bool MPVector::operator==(const MPVector& mpv) const {
	if (n != mpv.n) {
		return false;
	}
	for (int i = 0; i < n; ++i) {
		if (val[i] != mpv.val[i]) {
			return false;
		}
	}
	return true;
}

const bool MPVector::operator!=(const MPVector& mpv) const {
	return !(*this == mpv);
}

int MPVector::operator+(const MPVector& mpv) const {
	int resVal = NegInf;
	if (n != mpv.n)
		throw invdimex;

	for (int i = 0; i < n; ++i)
		if (val[i]!=NegInf && mpv.val[i]!=NegInf)
			resVal = max(resVal, val[i]+mpv.val[i]);

	return resVal;
}

MPVector& MPVector::operator+=(const int v) {
	for (int i = 0; i < n; ++i) {
		if (val[i] == NegInf || v == NegInf) {
			val[i] = NegInf;
		} else {
			val[i] += v;
		}
	}
	return *this;
}

const MPVector MPVector::operator+(const int v) const {
	return MPVector(*this) += v;
}

MPVector MPVector::Max(const MPVector& mpv) const {
	if (n != mpv.n) {
		throw invdimex;
	}

	MPVector retVal(*this);
	for (int i = 0; i < n; ++i) {
		retVal.Set(i, max(val[i],mpv.val[i]));
	}
	return retVal;
}

int MPVector::Get(const int idx) const {
	if (idx < 0 || idx >= n) {
		throw invdimex;
	}

	return val[idx];
}

void MPVector::Set(const int idx, const int v) {
	if (idx < 0 || idx >= n) {
		throw invdimex;
	}

	val[idx] = v;
}

const bool MPVector::operator<(const MPVector & mpv) const
{
	if (n != mpv.n) {
		throw invdimex;
	}

	for (int i = FirstClock; i < n; ++i) {
		if (val[i] < mpv.val[i]) {
			return true;
		}
	}
	return val[ConeIdx] < mpv.val[ConeIdx];
}

const bool MPVector::operator<=(const MPVector & mpv) const {
	return !(*this > mpv);
}

const bool MPVector::operator>=(const MPVector & mpv) const {
	return !(*this < mpv);
}

const bool MPVector::operator>(const MPVector & mpv) const {
	return mpv < *this;
}

int MPVector::GetDim() const {
	return n;
}

MPVector Max(const MPVector& lhs, const MPVector& rhs) {
	return lhs.Max(rhs);
}

const MPVector operator+(const int v, const MPVector& mpv) {
	return mpv + v;
}
