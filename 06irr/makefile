NAME      := irr
CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CXX       := ccache g++
LD_FLAGS  :=
LD_FLAGS  := -L/usr/local/lib -lQuantLib -lboost_unit_test_framework-mt
# -lboost_system-clang35-mt-1_56
# -lboost_thread-mt
CC_FLAGS  := -O2 -Wno-deprecated-declarations -std=c++11 -I/usr/local/include

${NAME}.exe: $(OBJ_FILES)
	${CXX} -o $@ $^ $(LD_FLAGS)

obj/%.o: %.cpp
	if [ ! -d obj ]; then mkdir obj; fi
	${CXX} $(CC_FLAGS) -c -o $@ $<

clean:
	if [ -d obj ]; then rm -fr obj; fi
	if [ -f ${NAME}.exe ]; then rm -fr ${NAME}.exe; fi

test: ${NAME}.exe
	./${NAME}.exe
