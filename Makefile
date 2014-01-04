# This file is part of the Electron Orbital Explorer. The Electron
# Orbital Explorer is distributed under the Simplified BSD License
# (also called the "BSD 2-Clause License"), in hopes that these
# rendering techniques might be used by other programmers in
# applications such as scientific visualization, video gaming, and so
# on. If you find value in this software and use its technologies for
# another purpose, I would love to hear back from you at bjthinks (at)
# gmail (dot) com. If you improve this software and agree to release
# your modifications under the below license, I encourage you to fork
# the development tree on github and push your modifications. The
# Electron Orbital Explorer's development URL is:
# https://github.com/bjthinks/orbital-explorer
# (This paragraph is not part of the software license and may be
# removed.)
#
# Copyright (c) 2013, Brian W. Johnson
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# + Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# + Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

OPT_OR_DEBUG = -O3

CXX = g++
CXXFLAGS := -pthread -Wall -Wshadow -Werror $(OPT_OR_DEBUG) $(shell sdl2-config --cflags) $(shell freetype-config --cflags)
LINKFLAGS := -pthread -lAntTweakBar $(shell sdl2-config --libs) $(shell freetype-config --libs)

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
	icon.o \
	font.o \
	font_data.o

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

shaders.cc: *.vert *.geom *.frag license.py wrap_shader.sh bin2string
	rm -f shaders.cc
	python3 -B license.py c >> shaders.cc
	echo >> shaders.cc
	for shader in *.vert *.geom *.frag ; do \
	  ./wrap_shader.sh $$shader >> shaders.cc ; \
	done

radial_data.cc: radial_analyzer.py license.py
	rm -f radial_data.cc
	python3 -B radial_analyzer.py > radial_data.cc

FONT = SourceSansPro-Regular.ttf
font_data.cc: $(FONT) license.py bin2string
	rm -f font_data.cc
	python3 -B license.py c >> font_data.cc
	echo >> font_data.cc
	echo '#include "font_data.hh"' >> font_data.cc
	echo >> font_data.cc
	./bin2string font_data unsigned < $(FONT) >> font_data.cc
	echo 'extern const size_t font_data_size = sizeof(font_data);' >> font_data.cc

.PHONY: clean
clean:
	rm -f *~ *.o $(PROG) $(TEST) bin2string

.PHONY: cleanall
cleanall: clean
	rm -f .*.d font_data.cc radial_data.cc shaders.cc

# Import dependences
-include $(OFILES:%.o=.%.d)
-include .unittests.d
