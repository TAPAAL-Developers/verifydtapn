compiler=g++
input=test.cpp ../src/Core/SymbolicMarking/*.cpp ../src/Core/TAPN/*.cpp
output=mptest
options=-Wall -Wno-write-strings -g
libs=udbm_debug
include=/usr/local/uppaal/include
libpath=/usr/local/uppaal/lib

all:
	$(compiler) -DDEBUG -DDBM_NORESIZE -I$(include) -L$(libpath) $(options) -o $(output) $(input) -l udbm_debug
	$(compiler) -DDEBUG -I$(include) -L$(libpath) $(options) -o $(output)_resize $(input) -l udbm_debug
