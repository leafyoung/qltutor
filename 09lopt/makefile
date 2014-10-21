NAME      := try
CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CXX       := clang++
# CXX       := g++
LD_FLAGS  := -L/usr/local/lib -lQuantLib  -static-libstdc++
# -lboost_system-mt -lboost_thread-mt -lboost_unit_test_framework-mt
CC_FLAGS  := -O0 -I/usr/local/include

# --std=c++11

#  -static-libgcc

${NAME}.exe: $(OBJ_FILES)
	${CXX} -o $@ $^ $(LD_FLAGS)

obj/%.o: %.cpp
	if [ ! -d obj ]; then mkdir obj; fi
	${CXX} $(CC_FLAGS) -c -o $@ $<

clean:
	if [ -d obj ]; then rm -fr obj; fi
	if [ -f ${NAME}.exe ]; then rm -fr ${NAME}.exe; fi

