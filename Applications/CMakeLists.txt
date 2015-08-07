PROJECT(StitchEngineApplications)

SET(APP_LBT_SRC
  main.cpp
)

SET(APP_TEST_BRDF_SRC
  main_testBRDFs.cpp
)

SET(APP_TEST_PROB_CALC_SRC
  main_testProbCalc.cpp
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

#===============================
#=== Test BRDFs target =========
#===============================
#Remember "-DSTITCHENGINE_CPP" flag when compiling cpp code directly into your app.
ADD_EXECUTABLE(testBRDFs ${APP_TEST_BRDF_SRC} ${CPP_LIB_SRC})
SET_TARGET_PROPERTIES(testBRDFs PROPERTIES COMPILE_FLAGS "-DSTITCHENGINE_CPP")
TARGET_LINK_LIBRARIES(testBRDFs ${Boost_LIBRARIES} ${OPENEXR_LIBRARIES})
IF(OPENSCENEGRAPH_FOUND)
    TARGET_LINK_LIBRARIES(testBRDFs ${OPENSCENEGRAPH_LIBRARIES})
ENDIF(OPENSCENEGRAPH_FOUND)
IF(NOT APPLE)  #Apple does not seem to have these.
    TARGET_LINK_LIBRARIES(testBRDFs rt)
ENDIF(NOT APPLE)
#===============================
#===============================

#===========================
#== Test probCalcTest app ==
#===========================
IF(OPENSCENEGRAPH_FOUND)
#Remember "-DSTITCHENGINE_CPP" flag when compiling cpp code directly into your app.
ADD_EXECUTABLE(probCalcTest ${APP_TEST_PROB_CALC_SRC} ${CPP_LIB_SRC})
SET_TARGET_PROPERTIES(probCalcTest PROPERTIES COMPILE_FLAGS "-DSTITCHENGINE_CPP")
TARGET_LINK_LIBRARIES(probCalcTest ${Boost_LIBRARIES} ${OPENEXR_LIBRARIES})
TARGET_LINK_LIBRARIES(probCalcTest ${OPENSCENEGRAPH_LIBRARIES})

IF(NOT APPLE)  #Apple does not seem to have these.
    TARGET_LINK_LIBRARIES(probCalcTest rt)
ENDIF(NOT APPLE)

ENDIF(OPENSCENEGRAPH_FOUND)
#===========================
#===========================

#========================
#=== LBT target =========
#========================
#Remember "-DSTITCHENGINE_CPP" flag when compiling cpp code directly into your app.
ADD_EXECUTABLE(lbt ${APP_LBT_SRC} ${CPP_LIB_SRC})
SET_TARGET_PROPERTIES(lbt PROPERTIES COMPILE_FLAGS "-DSTITCHENGINE_CPP")
TARGET_LINK_LIBRARIES(lbt ${Boost_LIBRARIES} ${OPENEXR_LIBRARIES})
IF(OPENSCENEGRAPH_FOUND)
TARGET_LINK_LIBRARIES(lbt ${OPENSCENEGRAPH_LIBRARIES})
ENDIF(OPENSCENEGRAPH_FOUND)
IF(NOT APPLE)  #Apple does not seem to have these.
TARGET_LINK_LIBRARIES(lbt rt)
ENDIF(NOT APPLE)
#========================
#========================

