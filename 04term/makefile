NAME      := term
CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS  := -L/usr/local/lib -lQuantLib -lboost_system-mt -lboost_thread-mt -lboost_unit_test_framework-mt
CC_FLAGS  := -O2 -I/usr/local/include
CPP       := ccache g++

${NAME}.exe: $(OBJ_FILES)
	${CPP} -o $@ $^ $(LD_FLAGS)

obj/%.o: %.cpp
	if [ ! -d obj ]; then mkdir obj; fi
	${CPP} $(CC_FLAGS) -c -o $@ $<

clean:
	if [ -d obj ]; then rm -fr obj; fi
	if [ -f ${NAME}.exe ]; then rm -fr ${NAME}.exe; fi

