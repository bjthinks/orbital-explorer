#!/bin/sh

shaderVarName=`echo ${1##*/} | sed -e 's/\.vert$/Vertex/' | sed -e 's/\.geom$/Geometry/' | sed -e 's/\.frag$/Fragment/'`ShaderSource

./bin2string $shaderVarName null < $1
