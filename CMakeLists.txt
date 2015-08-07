
# $Id: CMakeLists.txt 298 2015-03-25 13:00:40Z bernardt.duvenhage $
# 
#   CMakeLists.txt
#   StitchEngine
# 
#   Created by Bernardt Duvenhage on 2009/09/01.
#   Copyright $Date: 2015-03-25 15:00:40 +0200 (Wed, 25 Mar 2015) $ Bernardt Duvenhage. All rights reserved.
# 
#
# This file is part of StitchEngine.
# 
# StitchEngine is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# StitchEngine is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with StitchEngine.  If not, see <http://www.gnu.org/licenses/>.
#
#


PROJECT(StitchEngine)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(USE_CXX11 "Use the C++11 language dialect." ON)

IF(USE_CXX11)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_C_LANGUAGE_STANDARD "c11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_C_LIBRARY "libc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -g -Wall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall")
ELSE(USE_CXX11)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++98")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_C_LANGUAGE_STANDARD "c99")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_C_LIBRARY "libstdc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98 -g -Wall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wall")
ENDIF(USE_CXX11)

IF(USE_CXX11)
  ADD_DEFINITIONS(-DUSE_CXX11)
ENDIF(USE_CXX11)

OPTION(USE_VEC3_SSE "Use the Vec3 SSE optimisations which may or may not be faster than the code that the compiler can generate." OFF)

IF(USE_VEC3_SSE)
  ADD_DEFINITIONS(-DUSE_VEC3_SSE)
ENDIF(USE_VEC3_SSE)

ADD_DEFINITIONS(-DSTITCH_RAY_RADIANCE_PAYLOAD)

# compiler flags
IF(MSVC)
  # VS automatically defines NDEBUG in Release builds 
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CRT_SECURE_NO_DEPRECATE /D _WIN32_WINNT=0x0501")
ELSEIF(APPLE)
  SET(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for OSX" FORCE)
ELSE()
  #native is only supported by recent g++ compilers
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native -mtune=native -mfpmath=sse -Wall")
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -g -DNDEBUG -march=native -mtune=native -Wall")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall")
ENDIF()


SET(CMAKE_DEBUG_POSTFIX "d")


SET(STITCHENGINE_VERSION_MAJOR 0)
SET(STITCHENGINE_VERSION_MINOR 9)
SET(STITCHENGINE_VERSION "${STITCHENGINE_VERSION_MAJOR}.${STITCHENGINE_VERSION_MINOR}")
CONFIGURE_FILE(stitchengine_version.h.in stitchengine_version.h)


# Set a default build type for single-configuration
# CMake generators if no build type is set.
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release CACHE STRING
     "Choose the type of build, options are: Debug Release RelWithDebInfo."
     FORCE)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# There are some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})

OPTION(USE_OPENMP "Attempt to use OpenMP." OFF)

IF(USE_OPENMP)
#== Enable OpenMP ==
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

    ADD_DEFINITIONS(-DUSE_OPENMP)
endif()
ENDIF(USE_OPENMP)


IF (NOT USE_CXX11)
# === find Boost ===
SET(Boost_USE_STATIC_LIBS OFF CACHE BOOL "Link boost in statically")
SET(Boost_NO_SYSTEM_PATHS OFF CACHE BOOL "Search only BOOST_ROOT.")
SET(BOOST_ROOT "" CACHE PATH "Path to search for custom boost library. Empty to use default paths.")

MARK_AS_ADVANCED(FORCE ${CMAKE_PREFIX_PATH})
#SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_ADDITIONAL_VERSIONS "1.46")

FIND_PACKAGE(Boost 1.46 REQUIRED COMPONENTS filesystem thread system signals regex)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

IF (NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Cannot find boost, set BOOST_ROOT.")
ENDIF(NOT Boost_FOUND)
# ==================
ENDIF(NOT USE_CXX11)


#== Find OpenEXR ==
OPTION(USE_EXR "Use OpenEXR for saving the result." OFF)

IF(USE_EXR)
  FIND_PACKAGE(OpenEXR)
  IF(OPENEXR_FOUND)
    INCLUDE_DIRECTORIES(${OPENEXR_INCLUDE_PATHS})

    ADD_DEFINITIONS(-DUSE_OPENEXR)
  ENDIF(OPENEXR_FOUND)
ENDIF(USE_EXR)


#== Find OpenSceneGraph ==
OPTION(USE_OSG "Use Open Scene Graph for preview and viewing of result." OFF)

IF(USE_OSG)
  SET(OpenSceneGraph_MARK_AS_ADVANCED ON)
  FIND_PACKAGE(OpenSceneGraph 3.0.1 COMPONENTS osgViewer osgUtil osgGA osgDB osgText)
  IF(OPENSCENEGRAPH_FOUND)
    INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIR})
    LINK_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIRS}/../bin ${OPENSCENEGRAPH_LIBRARY_DIRS})

    ADD_DEFINITIONS(-DUSE_OSG)
  ENDIF(OPENSCENEGRAPH_FOUND)
ENDIF(USE_OSG)


#==
SET(SOURCES_BASE
	${CMAKE_BINARY_DIR}/stitchengine_version.h
	${CMAKE_SOURCE_DIR}/export.h

	${CMAKE_SOURCE_DIR}/Timer.h
	${CMAKE_SOURCE_DIR}/Timer.cpp

	${CMAKE_SOURCE_DIR}/EntryExit.h
	${CMAKE_SOURCE_DIR}/Intersection.h

	${CMAKE_SOURCE_DIR}/Material.h
	${CMAKE_SOURCE_DIR}/Material.cpp

	${CMAKE_SOURCE_DIR}/Light.h
	${CMAKE_SOURCE_DIR}/Light.cpp

	${CMAKE_SOURCE_DIR}/Beam.h
	${CMAKE_SOURCE_DIR}/Beam.cpp
	${CMAKE_SOURCE_DIR}/BeamTree.h
	${CMAKE_SOURCE_DIR}/BeamTree.cpp

	${CMAKE_SOURCE_DIR}/KNearestItems.h
	${CMAKE_SOURCE_DIR}/KDTree.h
	${CMAKE_SOURCE_DIR}/KDTree.cpp

	${CMAKE_SOURCE_DIR}/Photon.h
	${CMAKE_SOURCE_DIR}/Photon.cpp
	${CMAKE_SOURCE_DIR}/PhotonMap.h
	${CMAKE_SOURCE_DIR}/PhotonMap.cpp

	${CMAKE_SOURCE_DIR}/BoundingVolume.h
	${CMAKE_SOURCE_DIR}/BoundingVolume.cpp
	${CMAKE_SOURCE_DIR}/Object.h
	${CMAKE_SOURCE_DIR}/Object.cpp

	${CMAKE_SOURCE_DIR}/BallTree.h
	${CMAKE_SOURCE_DIR}/BallTree.cpp

	${CMAKE_SOURCE_DIR}/Scene.h
	${CMAKE_SOURCE_DIR}/Scene.cpp

	${CMAKE_SOURCE_DIR}/Camera.h
	${CMAKE_SOURCE_DIR}/Camera.cpp
	${CMAKE_SOURCE_DIR}/RadianceMap.h
	${CMAKE_SOURCE_DIR}/RadianceMap.cpp
	${CMAKE_SOURCE_DIR}/Renderer.h
	${CMAKE_SOURCE_DIR}/Renderer.cpp

	${CMAKE_SOURCE_DIR}/COPYING.LESSER.txt
	${CMAKE_SOURCE_DIR}/COPYING.txt
	${CMAKE_SOURCE_DIR}/ToDo.txt
	${CMAKE_SOURCE_DIR}/HowTo.txt
	${CMAKE_SOURCE_DIR}/CoffeeShake.txt
)

SET(SOURCES_IOUTILS
        ${CMAKE_SOURCE_DIR}/IOUtils/exr.h
        ${CMAKE_SOURCE_DIR}/IOUtils/exr.cpp
        ${CMAKE_SOURCE_DIR}/IOUtils/mdla.h
        ${CMAKE_SOURCE_DIR}/IOUtils/mdla.cpp
	${CMAKE_SOURCE_DIR}/IOUtils/ply.h
	${CMAKE_SOURCE_DIR}/IOUtils/ply.c
)

SET(SOURCES_MATH
	${CMAKE_SOURCE_DIR}/Math/MathUtil.h
	${CMAKE_SOURCE_DIR}/Math/MathUtil.cpp
	${CMAKE_SOURCE_DIR}/Math/GlobalRand.h
	${CMAKE_SOURCE_DIR}/Math/GlobalRand.cpp
	${CMAKE_SOURCE_DIR}/Math/IVec.h
	${CMAKE_SOURCE_DIR}/Math/Vec3.h
	${CMAKE_SOURCE_DIR}/Math/Vec3.cpp
	${CMAKE_SOURCE_DIR}/Math/Vec4.h
	${CMAKE_SOURCE_DIR}/Math/Vec4.cpp
	${CMAKE_SOURCE_DIR}/Math/VecN.h
	${CMAKE_SOURCE_DIR}/Math/VecN.cpp
	${CMAKE_SOURCE_DIR}/Math/Mat4.h
	${CMAKE_SOURCE_DIR}/Math/Mat4.cpp
	${CMAKE_SOURCE_DIR}/Math/Colour.h
	${CMAKE_SOURCE_DIR}/Math/Colour.cpp
	${CMAKE_SOURCE_DIR}/Math/Plane.h
	${CMAKE_SOURCE_DIR}/Math/Plane.cpp
	${CMAKE_SOURCE_DIR}/Math/Line.h
	${CMAKE_SOURCE_DIR}/Math/Line.cpp
	${CMAKE_SOURCE_DIR}/Math/Ray.h
)

IF(OPENSCENEGRAPH_FOUND)
SET(SOURCES_OSGUTILS
	${CMAKE_SOURCE_DIR}/OSGUtils/StitchOSG.h
	${CMAKE_SOURCE_DIR}/OSGUtils/StitchOSG.cpp
	${CMAKE_SOURCE_DIR}/OSGUtils/Viewer.h
	${CMAKE_SOURCE_DIR}/OSGUtils/Viewer.cpp
)
ENDIF(OPENSCENEGRAPH_FOUND)

SET(SOURCES_OBJECTS
	${CMAKE_SOURCE_DIR}/Objects/PolygonModel.h
	${CMAKE_SOURCE_DIR}/Objects/PolygonModel.cpp
	${CMAKE_SOURCE_DIR}/Objects/BrushModel.h
	${CMAKE_SOURCE_DIR}/Objects/BrushModel.cpp
)

SET(SOURCES_LIGHTS
	${CMAKE_SOURCE_DIR}/Lights/PointLight.h
	${CMAKE_SOURCE_DIR}/Lights/PointLight.cpp
	${CMAKE_SOURCE_DIR}/Lights/SpotLight.h
	${CMAKE_SOURCE_DIR}/Lights/SpotLight.cpp
)

SET(SOURCES_MATERIALS
	${CMAKE_SOURCE_DIR}/Materials/EmissiveMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/EmissiveMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/DiffuseMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/DiffuseMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/SpecularMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/SpecularMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/PhongMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/PhongMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/BlinnPhongMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/BlinnPhongMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/GlossyMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/GlossyMaterial.cpp
	${CMAKE_SOURCE_DIR}/Materials/GlossyTrnsMaterial.h
	${CMAKE_SOURCE_DIR}/Materials/GlossyTrnsMaterial.cpp
)

SET(SOURCES_RENDERERS
	${CMAKE_SOURCE_DIR}/Renderers/WhittedRenderer.h
	${CMAKE_SOURCE_DIR}/Renderers/WhittedRenderer.cpp
	${CMAKE_SOURCE_DIR}/Renderers/PathTraceRenderer.h
	${CMAKE_SOURCE_DIR}/Renderers/PathTraceRenderer.cpp
	${CMAKE_SOURCE_DIR}/Renderers/LightBeamRenderer.h
	${CMAKE_SOURCE_DIR}/Renderers/LightBeamRenderer.cpp
	${CMAKE_SOURCE_DIR}/Renderers/PhotonMapRenderer.h
	${CMAKE_SOURCE_DIR}/Renderers/PhotonMapRenderer.cpp
	${CMAKE_SOURCE_DIR}/Renderers/PhotonTraceRenderer.h
	${CMAKE_SOURCE_DIR}/Renderers/PhotonTraceRenderer.cpp
    ${CMAKE_SOURCE_DIR}/Renderers/LightFieldRenderer.h
    ${CMAKE_SOURCE_DIR}/Renderers/LightFieldRenderer.cpp
)


#=============================
#== Source groups for IDE ====
#=============================
SOURCE_GROUP(IOUtils FILES ${SOURCES_IOUTILS})
SOURCE_GROUP(Math FILES ${SOURCES_MATH})

IF(OPENSCENEGRAPH_FOUND)
  SOURCE_GROUP(OSGUtils FILES ${SOURCES_OSGUTILS})
ENDIF(OPENSCENEGRAPH_FOUND)

SOURCE_GROUP(Objects FILES ${SOURCES_OBJECTS})
SOURCE_GROUP(Lights FILES ${SOURCES_LIGHTS})
SOURCE_GROUP(Materials FILES ${SOURCES_MATERIALS})
SOURCE_GROUP(Renderers FILES ${SOURCES_RENDERERS})
#=============================
#=============================


INCLUDE_DIRECTORIES(/usr/local/include /opt/local/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
LINK_DIRECTORIES(${CMAKE_SOURCE_DIR} /usr/local/lib /opt/local/lib)


#==================
#== StitchEngine ==
#==================
SET(CPP_LIB_SRC
  ${SOURCES_BASE} 
  ${SOURCES_IOUTILS} 
  ${SOURCES_MATH} 
  ${SOURCES_OBJECTS} 
  ${SOURCES_LIGHTS} 
  ${SOURCES_MATERIALS} 
  ${SOURCES_RENDERERS}
)
IF(OPENSCENEGRAPH_FOUND)
  SET(CPP_LIB_SRC ${CPP_LIB_SRC} ${SOURCES_OSGUTILS})
ENDIF(OPENSCENEGRAPH_FOUND)
#==================
#==================


ADD_SUBDIRECTORY(Applications)


#====================
#== Doxygen target ==
#====================
OPTION(RUN_DOXYGEN "Generate the doxygen documentation." ON)

IF(RUN_DOXYGEN)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
  	  configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  	  add_custom_target(Doxygen ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generate Doxygen documentation." VERBATIM)
  else(DOXYGEN_FOUND)
        MESSAGE(WARNING "Doxygen not found!")
  endif(DOXYGEN_FOUND)
ENDIF(RUN_DOXYGEN)
#====================
#====================



