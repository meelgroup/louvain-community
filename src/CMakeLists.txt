# Copyright (c) 2017, Mate Soos
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

include_directories( ${PROJECT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

if (NOT WIN32)
    add_cxx_flag_if_supported("-Wno-bitfield-constant-conversion")
    #add_cxx_flag_if_supported("-Wduplicated-cond")
    #add_cxx_flag_if_supported("-Wduplicated-branches")
    add_cxx_flag_if_supported("-Wlogical-op")
    add_cxx_flag_if_supported("-Wrestrict")
    add_cxx_flag_if_supported("-Wnull-dereference")
    add_cxx_flag_if_supported("-Wjump-misses-init")
    add_cxx_flag_if_supported("-Wdouble-promotion")
    add_cxx_flag_if_supported("-Wshadow")
    add_cxx_flag_if_supported("-Wformat=2")
    add_cxx_flag_if_supported("-Wextra-semi")
    add_cxx_flag_if_supported("-pedantic")
    #add_cxx_flag_if_supported("-Wdeprecated")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

add_library(louvain_communities
    balmod.cpp
    condora.cpp
    devind.cpp
    devuni.cpp
    dp.cpp
    goldberg.cpp
    graph_binary.cpp
    graph_plain.cpp
    louvain.cpp
    modularity.cpp
    owzad.cpp
    quality.cpp
    shimalik.cpp
    zahn.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp
    louvain_communities.cpp
)


if (ENABLE_TESTING)
    add_library(louvain_communities_orig
        balmod.cpp
        condora.cpp
        devind.cpp
        devuni.cpp
        dp.cpp
        goldberg.cpp
        graph_binary.cpp
        graph_plain.cpp
        louvain.cpp
        modularity.cpp
        owzad.cpp
        quality.cpp
        shimalik.cpp
        zahn.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp
    )
    ####################
    ### BINARIES
    add_executable(comml-matrix
        main_matrix.cpp
    )

    add_executable(comml-convert
        main_convert.cpp
    )

    add_executable(comml-hierarchy
        main_hierarchy.cpp
    )

    add_executable(comml-louvain
        main_louvain.cpp
    )

    add_executable(example
        main_example.cpp
    )

    add_executable(example-libuse
        main_example2.cpp
    )



    ####################
    ### LINKING
    target_link_libraries(comml-louvain
        louvain_communities_orig
    )

    target_link_libraries(comml-convert
        louvain_communities_orig
    )

    target_link_libraries(example
        louvain_communities
    )

    target_link_libraries(example-libuse
        louvain_communities
    )

    ####################
    ### Target Properties
    set_target_properties(comml-matrix PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(comml-convert PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(comml-hierarchy PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(comml-louvain PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(example PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(example-libuse PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()






louvain_communities_add_public_header(
    louvain_communities ${CMAKE_CURRENT_SOURCE_DIR}/louvain_communities.h )

# -----------------------------------------------------------------------------
# Copy public headers into build directory include directory.
# The cryptominisat5Config.cmake we generate in the build directory depends on
# this.
# -----------------------------------------------------------------------------
set(HEADER_DEST "${PROJECT_BINARY_DIR}/include/louvain_communities")
add_custom_target(CopyPublicHeaders ALL)
get_target_property(louvain_communities_public_headers louvain_communities PUBLIC_HEADER)
foreach(public_header ${louvain_communities_public_headers})
    get_filename_component(HEADER_NAME ${public_header} NAME)
    add_custom_command(TARGET CopyPublicHeaders PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory
                               "${HEADER_DEST}"
                       COMMAND ${CMAKE_COMMAND} -E echo
                       "Copying ${HEADER_NAME} to ${HEADER_DEST}"
                       COMMAND ${CMAKE_COMMAND} -E
                           copy_if_different
                           ${public_header}
                           "${HEADER_DEST}"
                      )
endforeach()


install(TARGETS louvain_communities
    EXPORT ${LOUVAIN_COMMUNITIES_EXPORT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/louvain_communities"
)
