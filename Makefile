OPT_OR_DEBUG = -O3

CXX = g++
CXXFLAGS := -pthread -Wall -Wshadow -Werror $(OPT_OR_DEBUG) $(shell sdl2-config --cflags)
LINKFLAGS := -pthread -lAntTweakBar $(shell sdl2-config --libs)

ARCH = $(shell uname -s)
ifeq ($(ARCH),Linux)
LINKFLAGS += -lGLEW -lGL
else
ifeq ($(ARCH),Darwin)
LINKFLAGS += -framework OpenGL
else
$(error Unknown platform)
endif
endif

OFILES=\
	sdl_main.o \
	oopengl.o \
	render.o \
	viewport.o \
	camera.o \
	controls.o \
	transform.o \
	shaders.o \
	tetrahedralize.o \
	wavefunction.o \
	radial_data.o \
	util.o \
	solid.o \
	cloud.o \
	final.o \
	glprocs.o \
	myTwEventSDL20.o \
	icon.o

PROG = orbital-explorer
TEST = unittests

all: $(PROG)

$(PROG): $(OFILES)
	$(CXX) $(CXXFLAGS) $(OFILES) -o $@ $(LINKFLAGS)

$(TEST): unittests.o
	$(CXX) $(CXXFLAGS) unittests.o -o $@ $(LINKFLAGS) -lgtest -lgtest_main

bin2string: bin2string.o
	$(CXX) $(CXXFLAGS) bin2string.o -o $@ $(LINKFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(<:%.cc=.%.d) -c $<

shaders.cc: *.vert *.geom *.frag bin2string
	rm -f shaders.cc
	for shader in *.vert *.geom *.frag ; do \
	  ./wrap_shader.sh $$shader >> shaders.cc ; \
	done

radial_data.cc: radial_analyzer.py
	rm -f radial_data.cc
	python3 -B radial_analyzer.py > radial_data.cc

.PHONY: clean
clean:
	rm -f *~ *.o $(PROG) $(TEST) bin2string shaders.cc

.PHONY: cleanall
cleanall: clean
	rm -f .*.d radial_data.cc

# Import dependences
-include $(OFILES:%.o=.%.d)
-include .unittests.d
