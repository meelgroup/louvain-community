# Copyright (c) 2020, Mate Soos
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

enable_language( CXX )

message(STATUS "LIB directory is '${CMAKE_INSTALL_LIBDIR}'")
message(STATUS "BIN directory is '${CMAKE_INSTALL_BINDIR}'")

if(POLICY CMP0022)
    cmake_policy(SET CMP0022 NEW)
endif()

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

if(POLICY CMP0046)
    cmake_policy(SET CMP0046 NEW)
endif()

if(POLICY CMP0026)
    cmake_policy(SET CMP0026 NEW)
endif()

# -----------------------------------------------------------------------------
# Provide scripts dir for included cmakes to use
# -----------------------------------------------------------------------------
set(LOUVAIN_COMMUNITIES_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

include (GenerateExportHeader)
include (GNUInstallDirs)

# -----------------------------------------------------------------------------
# Make RelWithDebInfo the default build type if otherwise not set
# -----------------------------------------------------------------------------
set(build_types Debug Release RelWithDebInfo MinSizeRel)
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "You can choose the type of build, options are:${build_types}")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Options are ${build_types}"
        FORCE
    )

    # Provide drop down menu options in cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${build_types})
endif()
message(STATUS "Doing a ${CMAKE_BUILD_TYPE} build")

# -----------------------------------------------------------------------------
# Option to enable/disable assertions
# -----------------------------------------------------------------------------

# Filter out definition of NDEBUG from the default build configuration flags.
# We will add this ourselves if we want to disable assertions
foreach (build_config ${build_types})
    string(TOUPPER ${build_config} upper_case_build_config)
    foreach (language CXX C)
        set(VAR_TO_MODIFY "CMAKE_${language}_FLAGS_${upper_case_build_config}")
        string(REGEX REPLACE "(^| )[/-]D *NDEBUG($| )"
                             " "
                             replacement
                             "${${VAR_TO_MODIFY}}")
        #message("Original (${VAR_TO_MODIFY}) is ${${VAR_TO_MODIFY}} replacement is ${replacement}")
        set(${VAR_TO_MODIFY} "${replacement}" CACHE STRING "Default flags for ${build_config} configuration" FORCE)
    endforeach()
endforeach()

PROJECT(louvain_communities)

# contains some library search cmake scripts
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# generate JSON file of compile commands -- useful for code extension
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# static compilation
option(BUILD_SHARED_LIBS "Build the shared library" ON)
option(STATICCOMPILE "Compile to static executable" OFF)
if (STATICCOMPILE)
    set(BUILD_SHARED_LIBS OFF)
    set(Boost_USE_STATIC_LIBS ON)
endif()

option(SANITIZE "Use Clang sanitizers. You MUST use clang++ as the compiler for this to work" OFF)
if (SANITIZE)
    MESSAGE(WARNING " --Using clang sanitizers -- you MUST use clang++ or the compile WILL fail")
    SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -lasan -lubsan " )
    SET( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lasan -lubsan ")
    add_compile_options("-fsanitize=address")
    add_compile_options("-fsanitize=integer")
    add_compile_options("-fsanitize=undefined")

    add_compile_options("-fsanitize=null")
    add_compile_options("-fsanitize=alignment")
    #add_compile_options("-fno-sanitize-recover")

    add_compile_options("-fsanitize=return")
    add_compile_options("-fsanitize=bounds")
    add_compile_options("-fsanitize=float-divide-by-zero")
    add_compile_options("-fsanitize=integer-divide-by-zero")
    add_compile_options("-fsanitize=unsigned-integer-overflow")
    add_compile_options("-fsanitize=signed-integer-overflow")
    add_compile_options("-fsanitize=bool")
    add_compile_options("-fsanitize=enum")
    add_compile_options("-fsanitize=float-cast-overflow")

    #add_compile_options("-Weverything")
    #add_compile_options("-Wshorten-64-to-32")
    #add_compile_options("-Wweak-vtables")
    #add_compile_options("-Wsign-conversion")
    #add_compile_options("-Wconversion")
endif()

include(CheckCXXCompilerFlag)
macro(add_cxx_flag_if_supported flagname)
  check_cxx_compiler_flag("${flagname}" HAVE_FLAG_${flagname})

  if(HAVE_FLAG_${flagname})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flagname}" )
  endif()
endmacro()

option(ENABLE_ASSERTIONS "Build with assertions enabled" ON)
message(STATUS "build type is ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(ENABLE_ASSERTIONS OFF)
endif()

if (ENABLE_ASSERTIONS)
    # NDEBUG was already removed.
else()
    # Note this definition doesn't appear in the cache variables.
    add_definitions(-DNDEBUG)
    add_cxx_flag_if_supported("-fno-stack-protector")
    add_definitions(-D_FORTIFY_SOURCE=0)
endif()

# Note: O3 gives slight speed increase, 1 more solved from SAT Comp'14 @ 3600s
if (NOT MSVC)
    add_compile_options( -g)
    add_compile_options( -pthread )

    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:-O3>")
    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:-mtune=native>")

    add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
    add_compile_options("$<$<CONFIG:RELEASE>:-g0>")
    add_compile_options("$<$<CONFIG:RELEASE>:-DNDEBUG>")
    add_compile_options("$<$<CONFIG:RELEASE>:-mtune=native>")

    add_compile_options("$<$<CONFIG:DEBUG>:-O0>")

else()
    # see https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx for details
    # /ZI = include debug info
    # /Wall = all warnings

    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:/Ox>")
    add_compile_options("$<$<CONFIG:RELWITHDEBINFO>:/ZI>")

    add_compile_options("$<$<CONFIG:RELEASE>:/Ox>")
    add_compile_options("$<$<CONFIG:RELEASE>:/D>")
    add_compile_options("$<$<CONFIG:RELEASE>:/NDEBUG>")
    add_compile_options("$<$<CONFIG:RELEASE>:/ZI>")

    add_compile_options("$<$<CONFIG:DEBUG>:/Od>")

    if (STATICCOMPILE)
        # We statically link to reduce dependencies
        foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            # /MD -- Causes the application to use the multithread-specific and DLL-specific version of the run-time library.
            #        Defines _MT and _DLL and causes the compiler to place the library name MSVCRT.lib into the .obj file.
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MD")

            # /MDd	-- Defines _DEBUG, _MT, and _DLL and causes the application to use the debug multithread-specific and DLL-specific version of the run-time library.
            #          It also causes the compiler to place the library name MSVCRTD.lib into the .obj file.
            if(${flag_var} MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
            endif(${flag_var} MATCHES "/MDd")
        endforeach(flag_var)

        # Creates a multithreaded executable (static) file using LIBCMT.lib.
        add_compile_options(/MT)
    endif()

    # buffers security check
    add_compile_options(/GS)

    # Proper warning level
    add_compile_options(/W1)

    # Disable STL used in DLL-boundary warning
    add_compile_options(/wd4251)
    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)

    # Wall is MSVC's Weverything, so annoying unless used from the start
    # and with judiciously used warning disables
    # add_compile_options(/Wall)

    # /Za = only ansi C98 & C++11
    # /Za is not recommended for use, not tested, etc.
    # see: http://stackoverflow.com/questions/5489326/za-compiler-directive-does-not-compile-system-headers-in-vs2010
    # add_compile_options(/Za)

    add_compile_options(/fp:precise)

    # exception handling. s = The exception-handling model that catches C++ exceptions only and tells the compiler to assume that functions declared as extern "C" may throw an exception.
    # exception handling. c = If used with s (/EHsc), catches C++ exceptions only and tells the compiler to assume that functions declared as extern "C" never throw a C++ exception.
    add_compile_options(/EHsc)


    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /PDBCOMPRESS")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:1572864")

    #what does this do?
    set(DEF_INSTALL_CMAKE_DIR CMake)
endif()

option(ENABLE_TESTING "Enable testing" OFF)
option(COVERAGE "Build with coverage check" OFF)
if (COVERAGE AND NOT STATICCOMPILE)
    MESSAGE(FATAL_ERROR "Coverage can only be computed on static compilation")
endif()

if (NOT WIN32)
    add_cxx_flag_if_supported("-Wall")
    add_cxx_flag_if_supported("-Wextra")
    add_cxx_flag_if_supported("-Wunused")
    add_cxx_flag_if_supported("-Wsign-compare")
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
        add_cxx_flag_if_supported("-fno-omit-frame-pointer")
    endif()
    add_cxx_flag_if_supported("-Wtype-limits")
    add_cxx_flag_if_supported("-Wuninitialized")
    add_cxx_flag_if_supported("-Wno-deprecated")
    add_cxx_flag_if_supported("-Wstrict-aliasing")
    add_cxx_flag_if_supported("-Wpointer-arith")
    add_cxx_flag_if_supported("-Wheader-guard")
    if(NOT ENABLE_TESTING AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT COVERAGE)
        add_cxx_flag_if_supported("-fvisibility=hidden")
    endif()
    add_cxx_flag_if_supported("-Wpointer-arith")
    add_cxx_flag_if_supported("-Wformat-nonliteral")
    add_cxx_flag_if_supported("-Winit-self")
    add_cxx_flag_if_supported("-Wparentheses")
    add_cxx_flag_if_supported("-Wunreachable-code")
    add_cxx_flag_if_supported("-g")
    add_cxx_flag_if_supported("-ggdb3")
    add_compile_options("-fPIC")

    # add_cxx_flag_if_supported("-flto") # slow compile and not enough benefits
    #add_cxx_flag_if_supported("-fno-exceptions")
endif()

if (COVERAGE)
    add_compile_options("--coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "--coverage")
endif()


# -----------------------------------------------------------------------------
# Uncomment these for static compilation under Linux (messes up Valgrind)
# -----------------------------------------------------------------------------
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_EXE_LINKER_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -Wl,--discard-all -Wl,--build-id=sha1")
endif()

if ((${CMAKE_SYSTEM_NAME} MATCHES "Linux") OR (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    if(STATICCOMPILE)
        MESSAGE(STATUS "Compiling for static library use")
        if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static ")
        endif()

        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

        # removing -rdynamic that's automatically added
        foreach (language CXX C)
            set(VAR_TO_MODIFY "CMAKE_SHARED_LIBRARY_LINK_${language}_FLAGS")
            string(REGEX REPLACE "(^| )-rdynamic($| )"
                                 " "
                                 replacement
                                 "${${VAR_TO_MODIFY}}")
            #message("Original (${VAR_TO_MODIFY}) is ${${VAR_TO_MODIFY}} replacement is ${replacement}")
            set(${VAR_TO_MODIFY} "${replacement}" CACHE STRING "Default flags for ${build_config} configuration" FORCE)
        endforeach()
    else()
        add_definitions(-DBOOST_TEST_DYN_LINK)
        MESSAGE(STATUS "Compiling for dynamic library use")
    endif()
endif()

if(NOT MSVC)
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/louvain_communities)
endif()

# -----------------------------------------------------------------------------
# Add GIT version
# -----------------------------------------------------------------------------
function(SetVersionNumber PREFIX VERSION_MAJOR VERSION_MINOR VERSION_PATCH)
  set(${PREFIX}_VERSION_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
  set(${PREFIX}_VERSION_MINOR ${VERSION_MINOR} PARENT_SCOPE)
  set(${PREFIX}_VERSION_PATCH ${VERSION_PATCH} PARENT_SCOPE)
  set(${PREFIX}_VERSION
        "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
        PARENT_SCOPE)
endfunction()

find_program (GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_SHA1)
  MESSAGE(STATUS "GIT hash found: ${GIT_SHA1}")
else()
  set(GIT_SHA "GIT-hash-notfound")
endif()
set(LOUVAIN_COMMUNITIES_FULL_VERSION "1.0.0")

string(REPLACE "." ";" LOUVAIN_COMMUNITIES_FULL_VERSION_LIST ${LOUVAIN_COMMUNITIES_FULL_VERSION})
SetVersionNumber("PROJECT" ${LOUVAIN_COMMUNITIES_FULL_VERSION_LIST})
MESSAGE(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")
MESSAGE(STATUS "PROJECT_VERSION_MAJOR: ${PROJECT_VERSION_MAJOR}")
MESSAGE(STATUS "PROJECT_VERSION_MINOR: ${PROJECT_VERSION_MINOR}")
MESSAGE(STATUS "PROJECT_VERSION_PATCH: ${PROJECT_VERSION_PATCH}")

#query definitions
get_directory_property( DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
set(COMPILE_DEFINES)
foreach( d ${DirDefs} )
    # message( STATUS "Found Define: " ${d} )
    set(COMPILE_DEFINES "${COMPILE_DEFINES} -D${d}")
endforeach()
message(STATUS "All defines at startup: ${COMPILE_DEFINES}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

macro(louvain_communities_add_public_header LIBTARGET HEADER)
    get_target_property(EXISTING_PUBLIC_HEADERS ${LIBTARGET} PUBLIC_HEADER)
    if(EXISTING_PUBLIC_HEADERS)
        list(APPEND EXISTING_PUBLIC_HEADERS "${HEADER}")
    else()
        # Do not append to empty list
        set(EXISTING_PUBLIC_HEADERS "${HEADER}")
    endif()
    set_target_properties(
        ${LIBTARGET}
        PROPERTIES
        PUBLIC_HEADER "${EXISTING_PUBLIC_HEADERS}"
     )
endmacro()


# -----------------------------------------------------------------------------
# Provide an export name to be used by targets that wish to export themselves.
# -----------------------------------------------------------------------------
set(LOUVAIN_COMMUNITIES_EXPORT_NAME "louvain_communitiesTargets")

add_subdirectory(src src-louvain_communities)

# -----------------------------------------------------------------------------
# Add uninstall target for makefiles
# -----------------------------------------------------------------------------
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------
if (ENABLE_TESTING)
    enable_testing()

    #use valgrind
    message(STATUS "Testing is enabled")
    set(UNIT_TEST_EXE_SUFFIX "Tests" CACHE STRING "Suffix for Unit test executable")
    #add_subdirectory(tests)

else()
    message(WARNING "Testing is disabled")
endif()

# -----------------------------------------------------------------------------
# Export our targets so that other CMake based projects can interface with
# the build of louvain_communities in the build-tree
# -----------------------------------------------------------------------------
set(LOUVAIN_COMMUNITIES_TARGETS_FILENAME "louvain_communitiesTargets.cmake")
set(LOUVAIN_COMMUNITIES_CONFIG_FILENAME "louvain_communitiesConfig.cmake")

# Export targets
set(MY_TARGETS louvain_communities)
export(
    TARGETS ${MY_TARGETS}
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${LOUVAIN_COMMUNITIES_TARGETS_FILENAME}"
)

# Create louvain_communitiesConfig file
set(EXPORT_TYPE "Build-tree")
set(CONF_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include")
configure_file(louvain_communitiesConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${LOUVAIN_COMMUNITIES_CONFIG_FILENAME}" @ONLY
)

# Export this package to the CMake user package registry
# Now the user can just use find_package(louvain_communities) on their system
export(PACKAGE louvain_communities)

set(DEF_INSTALL_CMAKE_DIR lib/cmake/louvain_communities)
set(LOUVAIN_COMMUNITIES_INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
    "Installation directory for louvain_communities CMake files")

# Create louvain_communitiesConfig file
set(EXPORT_TYPE "installed")
set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
configure_file(louvain_communitiesConfig.cmake.in
   "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${LOUVAIN_COMMUNITIES_CONFIG_FILENAME}" @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${LOUVAIN_COMMUNITIES_CONFIG_FILENAME}"
    DESTINATION "${LOUVAIN_COMMUNITIES_INSTALL_CMAKE_DIR}"
)

# Install the export set for use with the install-tree
install(
    EXPORT ${LOUVAIN_COMMUNITIES_EXPORT_NAME}
    DESTINATION "${LOUVAIN_COMMUNITIES_INSTALL_CMAKE_DIR}"
)
