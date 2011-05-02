compiler=g++
input=test.cpp ../src/Core/SymbolicMarking/DiscreteMarking.cpp ../src/Core/SymbolicMarking/DiscretePart.cpp ../src/Core/SymbolicMarking/MP*.cpp ../src/Core/TAPN/*.cpp
output=mptest
options=-Wall -Wno-write-strings -g
libflags=-ludbm_debug
include=/usr/local/uppaal/include
libpath=/usr/local/uppaal/lib
-include config.mk

all: mptest

DEPS = \
	SymbolicMarking/DiscreteMarking.o \
	SymbolicMarking/DiscretePart.o \
	SymbolicMarking/MPVector.o \
	SymbolicMarking/MPPMarking.o \
	SymbolicMarking/MPPMarkingFactory.o \
	TAPN/InhibitorArc.o \
	TAPN/OutputArc.o \
	TAPN/Pairing.o \
	TAPN/TimedArcPetriNet.o \
	TAPN/TimedInputArc.o \
	TAPN/TimedPlace.o \
	TAPN/TimedTransition.o \
	TAPN/TimeInterval.o \
	TAPN/TimeInvariant.o \
	TAPN/TransportArc.o
	
define DEPS_template
objs/$(1): ../src/Core/$(1:.o=.cpp)
	$(compiler) -DDEBUG -I$(include) -L$(libpath) $(options) -c ../src/Core/$(1:.o=.cpp) -o objs/$(1)
endef 

$(foreach dep,$(DEPS),$(eval $(call DEPS_template,$(dep))))
OBJS = $(addprefix objs/,$(DEPS))
OBJS +=	objs/test.o
MODULE_DIRS += $(dir $(OBJS))
MODULE_DIRS := $(sort $(MODULE_DIRS))

clean:
	$(RM) $(addsuffix *.o,$(MODULE_DIRS))

objs/test.o: test.cpp
	$(compiler) -DDEBUG -I$(include) -L$(libpath) $(options) -c $< -o $@

mptest: $(OBJS)
	$(compiler) $(OBJS) -L$(libpath) $(options) $(libflags) -o $(output)