cmake_minimum_required(VERSION 2.6)

project(hybris)

# cmake needed modules
include(CheckIncludeFiles)

# needed programs, libraries and header files.
set( NEEDED_PROGRAMS  g++ flex bison make )
set( NEEDED_LIBRARIES dl pcre pthread ffi xml2 curl readline )
set( NEEDED_INCLUDES  dlfcn.h pcre.h pthread.h curl/curl.h readline/readline.h )
# check dependencies
foreach ( NEEDED_PROGRAM ${NEEDED_PROGRAMS} )
	find_program( ${NEEDED_PROGRAM}_FOUND NAMES ${NEEDED_PROGRAM} )	
	if ( NOT ${NEEDED_PROGRAM}_FOUND )
		message( FATAL_ERROR "-- Could not find ${NEEDED_PROGRAM}")
	else ( NOT ${NEEDED_PROGRAM}_FOUND )
		message( "-- Found ${NEEDED_PROGRAM} : ${${NEEDED_PROGRAM}_FOUND}" )
	endif ( NOT ${NEEDED_PROGRAM}_FOUND )
endforeach ( NEEDED_PROGRAM )

foreach ( NEEDED_LIBRARY ${NEEDED_LIBRARIES} )
	find_library( ${NEEDED_LIBRARY}_FOUND NAMES ${NEEDED_LIBRARY} )	
	if ( NOT ${NEEDED_LIBRARY}_FOUND )
		message( FATAL_ERROR "-- Could not find lib${NEEDED_LIBRARY}")
	else ( NOT ${NEEDED_LIBRARY}_FOUND )
		message( "-- Found lib${NEEDED_LIBRARY} : ${${NEEDED_LIBRARY}_FOUND}" )
	endif ( NOT ${NEEDED_LIBRARY}_FOUND )
endforeach ( NEEDED_LIBRARY )

foreach ( NEEDED_INCLUDE ${NEEDED_INCLUDES} )
	find_file( ${NEEDED_INCLUDE}_FOUND NAMES ${NEEDED_INCLUDE} PATH /usr/include /usr/local/include )
	if ( NOT ${NEEDED_INCLUDE}_FOUND )
		message( FATAL_ERROR "-- Could not find ${NEEDED_INCLUDE}")
	else ( NOT ${NEEDED_INCLUDE}_FOUND )
		message( "-- Found ${NEEDED_INCLUDE} : ${${NEEDED_INCLUDE}_FOUND}" )
	endif ( NOT ${NEEDED_INCLUDE}_FOUND )
endforeach ( NEEDED_INCLUDE )

# config variables
set( PREFIX usr )
set( AUTHOR "The Hybris Dev Team http://www.hybris-lang.org/" )
set( INC_PATH "/${PREFIX}/lib/hybris/include/" )
set( LIB_PATH "/${PREFIX}/lib/hybris/library/" )
set( MAJOR_VERSION 1 )
set( MINOR_VERSION 0 )
set( PATCH_LEVEL   0 )
set( VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL} beta 3" )
# common compilation flags
set( OPTIMIZATION "-O3 -pipe -fomit-frame-pointer -ffast-math" )
set( COMMON_CXXFLAGS "-w ${OPTIMIZATION}" )
# compute standard library compilation flags
execute_process( COMMAND xml2-config --cflags OUTPUT_VARIABLE LIBXML_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
execute_process( COMMAND pkg-config --cflags libffi OUTPUT_VARIABLE LIBFFI_CXXFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
set( STD_CXXFLAGS "${COMMON_CXXFLAGS} ${LIBXML_CXXFLAGS} ${LIBFFI_CXXFLAGS}" )

# libhybris.so sources
file( GLOB_RECURSE MAIN_SOURCES src/*.cpp )
# prevent bison and flex templates from being compiled, and main.cpp from being added to libhybris.so
list( REMOVE_ITEM MAIN_SOURCES ${CMAKE_BINARY_DIR}/src/lexer.l.cpp ${CMAKE_BINARY_DIR}/src/parser.y.cpp ${CMAKE_BINARY_DIR}/src/main.cpp )
set( LIB_SOURCES ${CMAKE_BINARY_DIR}/src/lexer.cpp
				 ${CMAKE_BINARY_DIR}/src/parser.cpp	
				 ${MAIN_SOURCES} )

# hybris headers to be installed
file( GLOB_RECURSE HEADERS include/*.* )
# hybris interpreter sources (to be linked with libhybris.so)
set( BIN_SOURCES src/main.cpp )
# stdlib sources
file( GLOB_RECURSE STD_SOURCES stdlib/*.cc )

# config.h generation
configure_file( include/config.h.in include/config.h )

# set main include directory (g++ -I flag)
include_directories( include )

# flex and bison commands to generate lexer and parser
add_custom_command(
    OUTPUT src/lexer.cpp 
    DEPENDS src/lexer.l.cpp 
    COMMAND flex -osrc/lexer.cpp src/lexer.l.cpp
)

add_custom_command(
    OUTPUT src/parser.cpp 
    DEPENDS src/parser.y.cpp 
    COMMAND bison -y --defines=include/parser.h src/parser.y.cpp -osrc/parser.cpp
)

# libhybris.so generation
add_library( libhybris SHARED ${LIB_SOURCES}  )
set_target_properties( libhybris PROPERTIES 
					   # create *nix style library versions + symbolic links
					   VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}" SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}"
					   # Remove the "lib" prefix
					   PREFIX ""
					   # Compile flags
					   COMPILE_FLAGS ${COMMON_CXXFLAGS}
					   # Output directory
					   RUNTIME_OUTPUT_DIRECTORY build/${PREFIX}/lib
					   LIBRARY_OUTPUT_DIRECTORY build/${PREFIX}/lib )
# Needed libraries
target_link_libraries( libhybris dl pcre pthread readline )

# hybris generation
add_executable( hybris ${BIN_SOURCES} ) 
set_target_properties( hybris PROPERTIES 
					   # Compile flags
					   COMPILE_FLAGS "${COMMON_CXXFLAGS} -Lbuild/${PREFIX}/lib"
					   # Output directory
					   RUNTIME_OUTPUT_DIRECTORY build/${PREFIX}/bin )
# Link with libhybris.so
target_link_libraries( hybris libhybris ) 

# Standard library
foreach( STD ${STD_SOURCES} )
	# Compute output directory
	string( REGEX REPLACE "^(.+)/stdlib/(.+)/[^/]+.cc$" "\\1/build/${PREFIX}/lib/hybris/library/\\2" OUTPUT_PATH ${STD} )
	# Compute target name
	string( REGEX REPLACE "^.+/([^/.]+).cc$" "\\1" LIB_NAME ${STD} ) 
	# Add the library to the targets
	add_library( ${LIB_NAME} SHARED ${STD} )
	# set its properties
	set_target_properties( ${LIB_NAME} PROPERTIES 
						   # remove lib prefix
						   PREFIX  "" 
					  	   # set output path
					   	   LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}
						   # set compile flags
						   COMPILE_FLAGS ${STD_CXXFLAGS} )
	# set its dependencies
	target_link_libraries( ${LIB_NAME}  
						   dl 
						   pcre 
						   curl
						   pthread
						   readline
						   ffi 
						   xml2
						   libhybris )
endforeach(STD)

# set files to install
install( FILES ${HEADERS} DESTINATION /${PREFIX}/include/hybris )
install( DIRECTORY stdinc/ DESTINATION /${PREFIX}/lib/hybris/include )
install( DIRECTORY build/${PREFIX}/lib/hybris/ DESTINATION /${PREFIX}/lib/hybris )
install( TARGETS   hybris DESTINATION /${PREFIX}/bin )
install( TARGETS   libhybris 
		 DESTINATION /${PREFIX}/lib 
		 PERMISSIONS
		 OWNER_READ 
		 OWNER_WRITE 
		 OWNER_EXECUTE
		 GROUP_READ 
		 GROUP_EXECUTE 
		 WORLD_READ 
		 WORLD_EXECUTE )

# Custom targets
add_custom_target( uninstall COMMAND xargs rm -rf < install_manifest.txt )
