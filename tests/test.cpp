#include "../src/Core/SymbolicMarking/MPPMarking.hpp"
#include "../src/Core/SymbolicMarking/MPPMarkingFactory.hpp"
#include <stdio.h>

using namespace VerifyTAPN;
using namespace VerifyTAPN::TAPN;

int tests = 0;
int pass = 0;
int fail = 0;
bool debug;

#define COLOR_RESET -1
#define COLOR_RED 1
#define COLOR_GREEN 2

char* testDesc;
#define STARTTEST(str) testDesc=str;
#define ENDPASS { setrgb(COLOR_GREEN); printf("%s...pass\n", testDesc); tests++; pass++; setrgb(COLOR_RESET);}
#define ENDFAIL { setrgb(COLOR_RED); printf("%s...fail\n", testDesc); tests++; fail++; setrgb(COLOR_RESET);}
#define ENDFAILMSG(msg) { setrgb(COLOR_RED); printf("%s...fail, %s\n", testDesc, msg); tests++; fail++; setrgb(COLOR_RESET);}
#define PASS(str) { STARTTEST(str); ENDPASS; }
#define FAIL(str) { STARTTEST(str); ENDFAIL; }
#define FAILMSG(str, msg) { STARTTEST(str); ENDFAILMSG(msg); }
#define TESTTRUE(var, str) if (var) PASS(str) else FAIL(str)
#define TESTFALSE(var, str) if (!var) PASS(str) else FAIL(str)
#define TESTEQ(var, val, str) if (var==val) PASS(str) else FAIL(str)
#define TESTNEQ(var, val, str) if (var!=val) PASS(str) else FAIL(str)

#define MPPTEST DiscretePart dp(std::vector<int>(10)); \
	MPPMarkingFactory f = MPPMarkingFactory(clocks); \
	MPPMarking::factory = &f;

#define NEWMARKING(m) MPPMarking* m = (MPPMarking *)f.InitialMarking(dp);
#define CREATEMARKING(v,w) MPPMarking(dp, clocks, v, w)

#define NEWVECVAL(val) MPVector(clocks, val)
#define NEWVEC NEWVECVAL(0)

#define INTERVAL(lower, upper) TimeInterval(false, lower, upper, false)

#define DEBUGON MPPMarking::debug = true;
#define DEBUGOFF MPPMarking::debug = debug;

int clocks = 10;

#ifdef __MINGW32__
#include <windows.h>

CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
void setrgb(int color) {
	switch (color) {
	case COLOR_RESET: // Reset
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ConsoleInfo.wAttributes);
		break;
	case COLOR_RED: // Red on Black
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		break;
	case COLOR_GREEN: // Green on Black
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		break;
	}
}
#else
void setrgb(int color) {
	if (color == -1)
	printf("\033[0m");
	else
	printf("\033[3%dm", color);
}
#endif

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

void TestRelation() {
	MPPTEST;
	MPVecSet v1, w1, v2, w2, w3;
	v1.insert(NEWVEC);
	w1.insert(NEWVEC);
	v2.insert(NEWVEC);
	w2.insert(NEWVEC);

	MPPMarking mpp1 = CREATEMARKING(v1,w1);
	MPPMarking mpp2 = CREATEMARKING(v2,w2);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestRelationEQ - convex+linear");
	TESTEQ(mpp2.Relation(mpp1), EQUAL, "TestRelationEQ - convex+linear");

	mpp1 = CREATEMARKING(v1,w3);
	mpp2 = CREATEMARKING(v2,w3);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestRelationEQ - convex");
	TESTEQ(mpp2.Relation(mpp1), EQUAL, "TestRelationEQ - convex");

	v1.insert(NEWVECVAL(5));

	mpp1 = CREATEMARKING(v1,w3);

	TESTEQ(mpp1.Relation(mpp2), SUPERSET, "TestRelationSUP - convex");
	TESTEQ(mpp2.Relation(mpp1), SUBSET, "TestRelationSUB - convex");

	v1.clear();
	v1.insert(NEWVECVAL(5));

	mpp1 = CREATEMARKING(v1,w3);

	TESTEQ(mpp1.Relation(mpp2), DIFFERENT, "TestRelationDIF - convex");
	TESTEQ(mpp2.Relation(mpp1), DIFFERENT, "TestRelationDIF - convex");

	v1.clear();
	v2.clear();
	w1.clear();

	v1.insert(NEWVEC);
	v2.insert(NEWVECVAL(7));
	w1.insert(NEWVEC);

	mpp1 = CREATEMARKING(v1,w1);
	mpp2 = CREATEMARKING(v2,w1);

	TESTEQ(mpp1.Relation(mpp2), SUPERSET, "TestRelationSUP - conv+lin");
	TESTEQ(mpp2.Relation(mpp1), SUBSET, "TestRelationSUB - conv+lin");

	v2.clear();

	MPVector mpv = NEWVEC;
	mpv.Set(1, 1);
	v2.insert(mpv);
	mpp2 = CREATEMARKING(v2,w1);
	TESTEQ(mpp1.Relation(mpp2), DIFFERENT, "TestRelationDIF - convex+linear");
	TESTEQ(mpp2.Relation(mpp1), DIFFERENT, "TestRelationDIF - convex+linear");

}

void TestReset() {
	MPPTEST;
	MPVecSet v, w;
	v.insert(NEWVEC);

	MPPMarking mpp1 = CREATEMARKING(v,w);
	MPPMarking mpp2 = CREATEMARKING(v,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex (0,0,..,0)");

	v.insert(NEWVECVAL(5));
	MPVecSet v2;
	v2.insert(NEWVEC);

	MPVector mpv = NEWVECVAL(5);
	mpv.Set(1,0);
	v2.insert(mpv);

	mpp1 = CREATEMARKING(v,w);
	mpp2 = CREATEMARKING(v2,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex 2 vector diagonal");

	mpp1.Reset(0);
	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset closed, convex");

	v.insert(NEWVECVAL(7));

	mpp1 = CREATEMARKING(v,w);

	v2.clear();
	v2.insert(NEWVEC);
	mpv= NEWVECVAL(7);
	mpv.Set(1,0);
	v2.insert(mpv);

	mpp2 = CREATEMARKING(v2,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex 3 vector diagonal");

	v.clear();
	v.insert(NEWVEC);
	w.insert(NEWVEC);

	mpp1 = CREATEMARKING(v,w);

	mpv=NEWVEC;
	mpv.Set(1,NegInf);
	MPVecSet w2;
	w2.insert(mpv);

	mpp2 = CREATEMARKING(v,w2);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset delayed (0,0,..,0)");
	mpp1.Reset(0);
	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset closed, linear");

}

void TestConstrain() {
	MPPTEST;
	MPVecSet v, v2, w;

	v.insert(NEWVEC);
	v.insert(NEWVECVAL(10));

	v2.insert(NEWVECVAL(4));
	v2.insert(NEWVECVAL(8));

	MPPMarking mpp1 = CREATEMARKING(v,w);
	MPPMarking mpp2 = CREATEMARKING(v2,w);

	mpp1.Constrain(0,INTERVAL(4,8));

	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain internal bounds convex");

	v.clear();
	v.insert(NEWVEC);

	w.insert(NEWVEC);

	mpp1 = CREATEMARKING(v,w);

	mpp1.Constrain(0,INTERVAL(4,8));

	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain internal bounds linear");

	v.clear();
	v.insert(NEWVECVAL(5));
	mpp1 = CREATEMARKING(v,w);
	mpp1.Constrain(0,INTERVAL(4,8));
	v2.clear();
	v2.insert(NEWVECVAL(5));
	v2.insert(NEWVECVAL(8));
	MPVecSet w2;
	mpp2 = CREATEMARKING(v2,w2);
	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain external lower bound");

	v.clear();
	v.insert(NEWVECVAL(4));
	v.insert(NEWVECVAL(6));
	mpp1 = CREATEMARKING(v,w2);
	mpp1.Constrain(0,INTERVAL(4,8));
	v2.clear();
	v2.insert(NEWVECVAL(4));
	v2.insert(NEWVECVAL(6));
	mpp2 = CREATEMARKING(v2,w2);
	TESTEQ(mpp1.Relation(mpp2),EQUAL,"TestConstrain external  upper bound");

	v.clear();
	v.insert(NEWVECVAL(5));
	mpp1 = CREATEMARKING(v,w2);
	mpp1.Constrain(0,INTERVAL(4,6));
	mpp2 = CREATEMARKING(v,w2);
	TESTEQ(mpp1.Relation(mpp2),EQUAL,"TestConstrain external upper and lower bound");
}

void TestSatisfy() {
	MPPTEST;
	NEWMARKING(m);

	m->Delay();
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(4,6)), "TestSatisfy delayed");

	m->Constrain(0, INTERVAL(4,6));

	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(4,6)), "TestSatisfy convex");
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(2,6)), "TestSatisfy outside lower");
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(4,8)), "TestSatisfy outside upper");
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(2, 8)), "TestSatisfy outside both");
	TESTFALSE(m->PotentiallySatisfies(0, INTERVAL(1, 3)), "TestSatisfy false");

	delete m;
}

void TestMPPMarking() {
	TestDelay();
	TestRelation();
	TestReset();
	TestConstrain();
	TestSatisfy();
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
	} catch (InvalidDimException) {
		ENDPASS;
	} catch (...) {
		ENDFAILMSG("wrong exception thrown");
	}

	/*MPVector::max()*
	 * 	invdimex-test, value-test, NegInf-test*/
	STARTTEST("Exception on max(), different dims");
	try {
		mpv3.Max(mpv5);
		ENDFAILMSG("no exception thrown");
	} catch (InvalidDimException) {
		ENDPASS;
	} catch (...) {
		ENDFAILMSG("wrong exception thrown");
	}

	for (int i = 0; i < mpv3.GetDim(); ++i) {
		mpv3.Set(i, i);
		mpv6.Set(i, i % 3);
	}

	MPVector xmpv(6), ympv(6);

	xmpv = mpv3.Max(mpv6); //xmpv = [0,1,2,3,4,5]

	TESTEQ(xmpv.Get(2),2,"max(2,2)");
	TESTEQ(xmpv.Get(5),5,"max(5,2)");

	xmpv.Set(0, NegInf);
	xmpv.Set(3, NegInf); //xmpv = [NegInf, 1, 2, NegInf, 4, 5]
	ympv = mpv3;
	ympv.Set(3, NegInf);
	ympv.Set(5, NegInf);
	ympv.Set(4, 4); //ympv = [0,1,2,NegInf,4,NegInf]
	xmpv = ympv.Max(xmpv); //xmpv = [0,1,2,NegiNF,4,5]

	TESTEQ(xmpv.Get(0),0,"max(0,NegInf)");
	TESTEQ(xmpv.Get(3),NegInf,"max(neginf,neginf)");
	TESTEQ(xmpv.Get(5),5,"max(5,neginf)");
}

int main(int argc, char** argv) {
#ifdef __MINGW32__
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleInfo);
#endif
	debug = (argc > 1);
	DiscreteMarking::debug = debug;
	TestMPVector();
	TestMPPMarking();

	printf("Test summary:\n%d tests total\n%d passed (%.2f%%)\n%d failed\n",
		tests, pass, pass / (float) tests * 100, fail);

	if (pass == tests) {
		setrgb(COLOR_GREEN);
		printf("\nAll tests passed\n");
		setrgb(COLOR_RESET);
	} else {
		setrgb(COLOR_RED);
		printf("\nSome tests failed\n");
		setrgb(COLOR_RESET);
	}

	return 0;
}
