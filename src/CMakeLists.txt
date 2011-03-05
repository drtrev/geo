# $Id$

cmake_minimum_required(VERSION 2.6)

PROJECT( geo )

# to debug, change the CMAKE_BUILD_TYPE in ccmake to Debug

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -DNOOGLFT")
MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )

INCLUDE_DIRECTORIES( ../include 
../../motlab-old/motlab/trunk/include
../../../motlab/motlab/trunk/include
../../motlab/motlab/trunk/include
/usr/not-backed-up/work/oglft-0.9/include/oglft
/usr/not-backed-up/work/oglft-0.9/bin/include/oglft
/usr/not-backed-up/work/openal-soft-1.7.411/include 
/usr/include/freetype2 /usr/local/include/oglft /usr/include/ImageMagick )

LINK_DIRECTORIES( /usr/not-backed-up/work/oglft-0.9/lib
../../motlab-old/motlab/trunk/build/lib
../../../motlab/motlab/trunk/build/lib
../../motlab/motlab/trunk/build/lib
/usr/not-backed-up/work/oglft-0.9/bin/lib
/usr/not-backed-up/work/openal-soft-1.7.411/build/
/usr/not-backed-up/openal-soft-1.7.411/build/ /usr/local/lib )

SET ( geo_SRCS clientcontrol controller directory graphics 
graphicsopengl input inputSDL inputX11 level main map outstd outverbose
pawn player servercontrol tess textures timer windowgen windowgensdl )

# note: network.h is detected as a dependency, as with all header files

#SET ( map_LIBS boost_regex openal X11 motlab glut SDL Magick++ OGLFT )
#SET ( geo_LIBS openal X11 motlab SDL Magick++ OGLFT )
SET ( geo_LIBS openal X11 motlab SDL GL GLU )

ADD_EXECUTABLE( geo ${geo_SRCS} )
TARGET_LINK_LIBRARIES( geo ${geo_LIBS} )
#SET_TARGET_PROPERTIES ( geo PROPERTIES LINK_FLAGS "-Wl,-Bdynamic -pthread" ) # for vrj

