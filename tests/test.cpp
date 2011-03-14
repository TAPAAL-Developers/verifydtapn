#include "../src/Core/SymbolicMarking/MPPMarking.hpp"
#include "../src/Core/SymbolicMarking/MPPMarkingFactory.hpp"
#include <stdio.h>

using namespace VerifyTAPN;
using namespace VerifyTAPN::TAPN;

int tests=0;
int pass=0;
int fail=0;
bool debug;

#define STARTTEST(str) printf("%s", str);
#define ENDPASS { printf("...pass\n"); tests++; pass++; }
#define ENDFAIL { printf("...fail\n"); tests++; fail++; }
#define ENDFAILMSG(msg) { printf("...fail, %s\n", msg); tests++; fail++; }
#define PASS(str) { STARTTEST(str); ENDPASS; }
#define FAIL(str) { STARTTEST(str); ENDFAIL; }
#define FAILMSG(str, msg) { STARTTEST(str); ENDFAILMSG(msg); }
#define TESTTRUE(var, str) if (var) PASS(str) else FAIL(str)
#define TESTFALSE(var, str) if (!var) PASS(str) else FAIL(str)
#define TESTEQ(var, val, str) if (var==val) PASS(str) else FAIL(str)
#define TESTNEQ(var, val, str) if (var!=val) PASS(str) else FAIL(str)

#define MPPTEST DiscretePart dp; \
	MPPMarkingFactory f = MPPMarkingFactory(clocks); \
	MPPMarking::factory = &f;

#define NEWMARKING(m) MPPMarking* m = (MPPMarking *)f.InitialMarking(dp);
#define CREATEMARKING(v,w) MPPMarking(dp, clocks, v, w)

#define NEWVECVAL(val) MPVector(clocks, val)
#define NEWVEC NEWVECVAL(0)

#define DEBUGON MPPMarking::debug = true;
#define DEBUGOFF MPPMarking::debug = debug;

int clocks = 10;

void TestDelay() {
	MPPTEST;
	MPVecSet v, w;
	v.insert(NEWVEC);
	w.insert(NEWVEC);
	MPPMarking expected = CREATEMARKING(v,w);

	NEWMARKING(m);
	m->Delay();

	TESTEQ(m->Relation(expected), EQUAL, "TestDelay");
	delete m;
}

void TestMPPMarking() {
	TestDelay();
}

void TestMPVector() {
	MPVector mpv1, mpv2(mpv1), mpv3(6), mpv4(mpv3), mpv5(5), mpv6(6);

	/*MPVector constructor tests, equality/inequality + invdimex*/
	TESTEQ(mpv3,mpv4,"copy constr");
	TESTNEQ(mpv1,mpv3,"inequality test");
	TESTEQ(mpv1,mpv2,"copy constr base");

	STARTTEST("Exception on dim=0 constructor");
	try {
		MPVector mpv5(0);
		ENDFAILMSG("no exception thrown");
	} catch(InvalidDimException) {
		ENDPASS;
	} catch(...) {
		ENDFAILMSG("wrong exception thrown");
	}

	/*MPVector::max()*
	 * 	invdimex-test, value-test, NegInf-test*/
	STARTTEST("Exception on max(), different dims");
	try {
		mpv3.Max(mpv5);
		ENDFAILMSG("no exception thrown");
	} catch(InvalidDimException) {
		ENDPASS;
	} catch(...) {
		ENDFAILMSG("wrong exception thrown");
	}

	for(int i=0; i<mpv3.GetDim(); ++i) {
		mpv3.Set(i, i);
		mpv6.Set(i, i%3);
	}

	MPVector xmpv(6), ympv(6);

	xmpv=mpv3.Max(mpv6); //xmpv = [0,1,2,3,4,5]

	TESTEQ(xmpv.Get(2),2,"max(2,2)");
	TESTEQ(xmpv.Get(5),5,"max(5,2)");

	xmpv.Set(0,NegInf);
	xmpv.Set(3,NegInf); //xmpv = [NegInf, 1, 2, NegInf, 4, 5]
	ympv=mpv3;
	ympv.Set(3,NegInf);
	ympv.Set(5,NegInf);
	ympv.Set(4,4); //ympv = [0,1,2,NegInf,4,NegInf]
	xmpv=ympv.Max(xmpv); //xmpv = [0,1,2,NegiNF,4,5]

	TESTEQ(xmpv.Get(0),0,"max(0,NegInf)");
	TESTEQ(xmpv.Get(3),NegInf,"max(neginf,neginf)");
	TESTEQ(xmpv.Get(5),5,"max(5,neginf)");
}

int main(int argc, char** argv) {
	debug = (argc > 1);
	MPPMarking::debug = debug;
	TestMPVector();
	TestMPPMarking();

	printf("Test summary:\n%d tests total\n%d passed (%.2f%%)\n%d failed\n", tests, pass, pass/(float)tests*100, fail);

	return 0;
}
