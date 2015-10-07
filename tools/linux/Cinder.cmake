
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset    "${Esc}[m"     )
  set(ColorBold     "${Esc}[1m"    )
  set(Red           "${Esc}[31m"   )
  set(Green         "${Esc}[32m"   )
  set(Yellow        "${Esc}[33m"   )
  set(Blue          "${Esc}[34m"   )
  set(Magenta       "${Esc}[35m"   )
  set(Cyan          "${Esc}[36m"   )
  set(White         "${Esc}[37m"   )
  set(BoldRed       "${Esc}[1;31m" )
  set(BoldGreen     "${Esc}[1;32m" )
  set(BoldYellow    "${Esc}[1;33m" )
  set(BoldBlue      "${Esc}[1;34m" )
  set(BoldMagenta   "${Esc}[1;35m" )
  set(BoldCyan      "${Esc}[1;36m" )
  set(BoldWhite     "${Esc}[1;37m" )
endif()

execute_process( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE CINDER_ARCH )

# Include Directories
set( CINDER_INC_DIR ${CINDER_DIR}/include )
include_directories( 
    ${CINDER_INC_DIR} 
)

if( CINDER_LINUX_EGL_RPI2 )
	include_directories( 
		/opt/vc/include
		/opt/vc/include/interface/vcos/pthreads
	)
	link_directories( /opt/vc/lib )
elseif()
	include_directories( ${CINDER_INC_DIR}/glfw )
endif()

# Source Directories
set( CINDER_SRC_DIR ${CINDER_DIR}/src )

# Library Directories
set( CINDER_LIB_DIR ${CINDER_DIR}/lib/linux/${CINDER_ARCH} )

set( CINDER_TOOLCHAIN_CLANG true )

if( CINDER_TOOLCHAIN_CLANG )
    set(CMAKE_TOOLCHAIN_PREFIX 					"llvm-"		CACHE STRING "" FORCE ) 
    set(CMAKE_C_COMPILER                      	"clang"		CACHE STRING "" FORCE )
    set(CMAKE_CXX_COMPILER                    	"clang++"	CACHE STRING "" FORCE )

    set(CMAKE_C_FLAGS_INIT                    	"-Wall -std=c99"	CACHE STRING "" FORCE )
    set(CMAKE_C_FLAGS_DEBUG_INIT              	"-g"				CACHE STRING "" FORCE )
    set(CMAKE_C_FLAGS_MINSIZEREL_INIT         	"-Os -DNDEBUG"		CACHE STRING "" FORCE )
    set(CMAKE_C_FLAGS_RELEASE_INIT            	"-O4 -DNDEBUG"		CACHE STRING "" FORCE )
    set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT     	"-O2 -g"			CACHE STRING "" FORCE )
    set(CMAKE_C_FLAGS                         	"${CMAKE_C_FLAGS} -fmessage-length=0"	CACHE STRING "" FORCE )

    set(CMAKE_CXX_FLAGS_INIT                  	"-Wall"			CACHE STRING "" FORCE )
    set(CMAKE_CXX_FLAGS_DEBUG_INIT            	"-g"			CACHE STRING "" FORCE )
    set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT       	"-Os -DNDEBUG"	CACHE STRING "" FORCE )
    set(CMAKE_CXX_FLAGS_RELEASE_INIT          	"-O4 -DNDEBUG"	CACHE STRING "" FORCE )
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT   	"-O2 -g"		CACHE STRING "" FORCE )
    set(CMAKE_CCC_FLAGS                       	"${CMAKE_C_FLAGS} -fmessage-length=0"	CACHE STRING "" FORCE )
endif()

set( CXX_FLAGS "-stdlib=libstdc++ -std=c++11 -Wno-reorder -Wno-unused-private-field -Wno-unused-local-typedef" CACHE STRING "" FORCE )
set( CMAKE_CXX_FLAGS_DEBUG    "${CXX_FLAGS} -g -fexceptions -frtti" 				CACHE STRING "" FORCE )
set( CMAKE_CXX_FLAGS_RELEASE  "${CXX_FLAGS} -Os -fexceptions -frtti -ffast-math" 	CACHE STRING "" FORCE )

if( CINDER_GL_ES_2 )
	set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}   -DCINDER_GL_ES_2" )
	set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -DCINDER_GL_ES_2" )
	list( APPEND CINDER_GL_LIBS EGL GLESv2 )
	set( CINDER_LIB_SUFFIX "-es2" )
elseif( CINDER_GL_ES_3 )
	set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}   -DCINDER_GL_ES_3" )
	set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -DCINDER_GL_ES_3" )
	list( APPEND CINDER_GL_LIBS EGL GLESv2 )
	set( CINDER_LIB_SUFFIX "-es3" )
elseif( CINDER_GL_ES_3_1 )
	set( CMAKE_CXX_FLAGS_DEBUG    "${CMAKE_CXX_FLAGS_DEBUG}   -DCINDER_GL_ES_3_1" )
	set( CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -DCINDER_GL_ES_3_1" )
	list( APPEND CINDER_GL_LIBS EGL GLESv2 )
	set( CINDER_LIB_SUFFIX "-es3" )
else()
	list( APPEND CINDER_GL_LIBS GL )
endif()
