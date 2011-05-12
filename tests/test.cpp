#include "../src/Core/SymbolicMarking/MPPMarking.hpp"
#include "../src/Core/SymbolicMarking/MPPMarkingFactory.hpp"
#include "../src/Core/TAPN/TimedArcPetriNet.hpp"
#include <stdio.h>
#include <boost/shared_ptr.hpp>

using namespace VerifyTAPN;
using namespace VerifyTAPN::TAPN;

int tests = 0;
int pass = 0;
int fail = 0;
bool debug;
TimedArcPetriNet* tapn;
boost::shared_ptr<TimedArcPetriNet> tapn_ptr;

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

#define MPPTEST DiscretePart dp = DiscretePart(std::vector<int>(clocks)); \
	MPPMarkingFactory<MPVector> f = MPPMarkingFactory<MPVector>(tapn_ptr); \

#define NEWMARKING(m) MPPMarking<MPVector>* m = (MPPMarking<MPVector> *)f.InitialMarking(std::vector<int>(clocks));
#define CREATEMARKING(v,w) MPPMarking<MPVector>(dp, v, w)

#define NEWVECVAL(val) MPVector(clocks, val)
#define NEWVEC NEWVECVAL(0)

#define INTERVAL(lower, upper) TimeInterval(false, lower, upper, false)

#define DEBUGON DiscreteMarking::debug = true;
#define DEBUGOFF DiscreteMarking::debug = debug;

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
	MPPMarking<MPVector>::MPVecSet v, w;
	v.push_back(NEWVEC);
	w.push_back(NEWVEC);
	MPPMarking<MPVector> expected = CREATEMARKING(v,w);
	NEWMARKING(m);
	m->Delay();

	TESTEQ(m->Relation(expected), EQUAL, "TestDelay");
	delete m;
}

void TestRelation() {
	MPPTEST;
	MPPMarking<MPVector>::MPVecSet v1, w1, v2, w2, w3;
	v1.push_back(NEWVEC);
	w1.push_back(NEWVEC);
	v2.push_back(NEWVEC);
	w2.push_back(NEWVEC);

	MPPMarking<MPVector> mpp1 = CREATEMARKING(v1,w1);
	MPPMarking<MPVector> mpp2 = CREATEMARKING(v2,w2);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestRelationEQ - convex+linear");
	TESTEQ(mpp2.Relation(mpp1), EQUAL, "TestRelationEQ - convex+linear");

	mpp1 = CREATEMARKING(v1,w3);
	mpp2 = CREATEMARKING(v2,w3);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestRelationEQ - convex");
	TESTEQ(mpp2.Relation(mpp1), EQUAL, "TestRelationEQ - convex");

	v1.push_back(NEWVECVAL(5));

	mpp1 = CREATEMARKING(v1,w3);

	TESTEQ(mpp1.Relation(mpp2), SUPERSET, "TestRelationSUP - convex");
	TESTEQ(mpp2.Relation(mpp1), SUBSET, "TestRelationSUB - convex");

	v1.clear();
	v1.push_back(NEWVECVAL(5));

	mpp1 = CREATEMARKING(v1,w3);

	TESTEQ(mpp1.Relation(mpp2), DIFFERENT, "TestRelationDIF - convex");
	TESTEQ(mpp2.Relation(mpp1), DIFFERENT, "TestRelationDIF - convex");

	v1.clear();
	v2.clear();
	w1.clear();

	v1.push_back(NEWVEC);
	v2.push_back(NEWVECVAL(7));
	w1.push_back(NEWVEC);

	mpp1 = CREATEMARKING(v1,w1);
	mpp2 = CREATEMARKING(v2,w1);

	TESTEQ(mpp1.Relation(mpp2), SUPERSET, "TestRelationSUP - conv+lin");
	TESTEQ(mpp2.Relation(mpp1), SUBSET, "TestRelationSUB - conv+lin");

	v2.clear();

	MPVector mpv = NEWVEC;
	mpv.Set(1, 1);
	v2.push_back(mpv);
	mpp2 = CREATEMARKING(v2,w1);
	TESTEQ(mpp1.Relation(mpp2), DIFFERENT, "TestRelationDIF - convex+linear");
	TESTEQ(mpp2.Relation(mpp1), DIFFERENT, "TestRelationDIF - convex+linear");

}

void TestReset() {
	MPPTEST;
	MPPMarking<MPVector>::MPVecSet v, w;
	v.push_back(NEWVEC);

	MPPMarking<MPVector> mpp1 = CREATEMARKING(v,w);
	MPPMarking<MPVector> mpp2 = CREATEMARKING(v,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex (0,0,..,0)");

	v.push_back(NEWVECVAL(5));
	MPPMarking<MPVector>::MPVecSet v2;
	v2.push_back(NEWVEC);

	MPVector mpv = NEWVECVAL(5);
	mpv.Set(1,0);
	v2.push_back(mpv);

	mpp1 = CREATEMARKING(v,w);
	mpp2 = CREATEMARKING(v2,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex 2 vector diagonal");

	mpp1.Reset(0);
	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset closed, convex");

	v.push_back(NEWVECVAL(7));

	mpp1 = CREATEMARKING(v,w);

	v2.clear();
	v2.push_back(NEWVEC);
	mpv= NEWVECVAL(7);
	mpv.Set(1,0);
	v2.push_back(mpv);

	mpp2 = CREATEMARKING(v2,w);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset convex 3 vector diagonal");

	v.clear();
	v.push_back(NEWVEC);
	w.push_back(NEWVEC);

	mpp1 = CREATEMARKING(v,w);

	mpv=NEWVEC;
	mpv.Set(1,NegInf);
	MPPMarking<MPVector>::MPVecSet w2;
	w2.push_back(mpv);

	mpp2 = CREATEMARKING(v,w2);

	mpp1.Reset(0);

	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset delayed (0,0,..,0)");
	mpp1.Reset(0);
	TESTEQ(mpp1.Relation(mpp2), EQUAL, "TestReset closed, linear");

}

void TestConstrain() {
	MPPTEST;
	MPPMarking<MPVector>::MPVecSet v, v2, w;

	v.push_back(NEWVEC);
	v.push_back(NEWVECVAL(10));

	v2.push_back(NEWVECVAL(4));
	v2.push_back(NEWVECVAL(8));

	MPPMarking<MPVector> mpp1 = CREATEMARKING(v,w);
	MPPMarking<MPVector> mpp2 = CREATEMARKING(v2,w);

	mpp1.Constrain(0,INTERVAL(4,8));

	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain internal bounds convex");

	v.clear();
	v.push_back(NEWVEC);

	w.push_back(NEWVEC);

	mpp1 = CREATEMARKING(v,w);

	mpp1.Constrain(0,INTERVAL(4,8));

	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain internal bounds linear");

	v.clear();
	v.push_back(NEWVECVAL(5));
	mpp1 = CREATEMARKING(v,w);
	mpp1.Constrain(0,INTERVAL(4,8));
	v2.clear();
	v2.push_back(NEWVECVAL(5));
	v2.push_back(NEWVECVAL(8));
	MPPMarking<MPVector>::MPVecSet w2;
	mpp2 = CREATEMARKING(v2,w2);
	TESTEQ(mpp1.Relation(mpp2),EQUAL, "TestConstrain external lower bound");

	v.clear();
	v.push_back(NEWVECVAL(4));
	v.push_back(NEWVECVAL(6));
	mpp1 = CREATEMARKING(v,w2);
	mpp1.Constrain(0,INTERVAL(4,8));
	v2.clear();
	v2.push_back(NEWVECVAL(4));
	v2.push_back(NEWVECVAL(6));
	mpp2 = CREATEMARKING(v2,w2);
	TESTEQ(mpp1.Relation(mpp2),EQUAL,"TestConstrain external  upper bound");

	v.clear();
	v.push_back(NEWVECVAL(5));
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
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(2,4)), "TestSatisfy exact upper");
	TESTTRUE(m->PotentiallySatisfies(0, INTERVAL(6,8)), "TestSatisfy exact lower");
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

template <typename MPVector>
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

void CreateNet() {
	TimedPlace::Vector places;
	TimedTransition::Vector transitions;
	TimedInputArc::Vector inputArcs;
	OutputArc::Vector outputArcs;
	TransportArc::Vector transportArcs;
	InhibitorArc::Vector inhibitorArcs;

	TimeInvariant timeInvariant = TimeInvariant::CreateFor("inf");
	places.push_back(boost::make_shared<TimedPlace>("", "", timeInvariant));
	//tapn = new TimedArcPetriNet();
	tapn_ptr = boost::make_shared<TimedArcPetriNet>(places, transitions, inputArcs, outputArcs, transportArcs, inhibitorArcs);
}

int main(int argc, char** argv) {
#ifdef __MINGW32__
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleInfo);
#endif
	CreateNet();
	debug = (argc > 1);
	DiscreteMarking::debug = debug;
	try {
		std::cout << "Testing MPVector\n";
		TestMPVector<MPVector>();
		std::cout << "Testing MPVectorCOW\n";
		TestMPVector<MPVectorCOW>();
		std::cout << "Testing MPPMarking\n";
		TestMPPMarking();
	} catch (std::exception& e) {
		std::cout << e.what();
	}

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

//	delete tapn;
	return 0;
}
