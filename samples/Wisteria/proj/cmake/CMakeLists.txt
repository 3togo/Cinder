cmake_minimum_required( VERSION 2.8 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( Wisteria )

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

ci_make_app(
	CINDER_PATH ${CINDER_PATH}
    
    SOURCES     ${APP_PATH}/src/WisteriaApp.cpp
	SOURCES     ${CINDER_PATH}/blocks/Cairo/src/Cairo.cpp
	
    INCLUDES    ${CINDER_PATH}/blocks/Cairo/include
    INCLUDES    ${APP_PATH}/include
)
