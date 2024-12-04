cmake_minimum_required(VERSION 3.15)

project(ldp_abst)

# define library version
set(LDP_ABST_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(LDP_ABST_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(LDP_ABST_VERSION ${LDP_ABST_VERSION_MAJOR}.${LDP_ABST_VERSION_MINOR} CACHE STRING "version" FORCE)

# from https://foonathan.net/2016/03/cmake-install/
set(include_dest "include/ldp_abst-${LDP_ABST_VERSION}")
set(main_lib_dest "lib/ldp_abst-${LDP_ABST_VERSION}")

# for config.h file since it may be different depending on whether we are doing Debug/Release/etc
set(lib_dest "${main_lib_dest}/${CMAKE_BUILD_TYPE}")

# don't look back
set(CMAKE_CXX_STANDARD 17)

# cross compiling doesn't work so well with the tests, so make them optional
if (BUILD_TESTING)
    option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
    add_subdirectory("thirdparty/googletest-1.14.0")
    add_subdirectory("tests")
else()
    # if not building the tests, then we also want to skip the compile test since we will be using a cross-compiler
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

add_subdirectory("src")

configure_file(ldp_abst-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/ldp_abst-config-version.cmake @ONLY)

install(FILES ldp_abst-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/ldp_abst-config-version.cmake DESTINATION ${main_lib_dest})
install(EXPORT ldp_abst FILE ldp_abst.cmake DESTINATION "${lib_dest}")
