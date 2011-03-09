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
	n = dim+2; //Add 2 to leave room for simpler poly/cone conversion and zero clock
	val = new int[n];
	for (int i = 0; i < n; i++)
		val[i] = defVal;
	val[ConeIdx] = NegInf;
	val[ZeroIdx] = 0;
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

MPVector& MPVector::operator+=(const MPVector& mpv) {
	if (n != mpv.n) {
		throw invdimex;
	}
	for (int i = 0; i < n; ++i) {
		if (val[i] == NegInf || mpv.val[i] == NegInf) {
			val[i] = NegInf;
		} else {
			val[i] += mpv.val[i];
		}
	}
	return *this;
}

const MPVector MPVector::operator+(const MPVector& mpv) const {

	return MPVector(*this) += mpv;
}

MPVector MPVector::Max(const MPVector& mpv) const {
	if (n != mpv.n) {
		throw invdimex;
	}

	MPVector retVal(n);
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

	for (int i = 0; i < n; ++i) {
		if (val[i] < mpv.val[i]) {
			return true;
		}
	}
	return false;
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
