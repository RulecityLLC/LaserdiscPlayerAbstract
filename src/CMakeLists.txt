
# for convenience
set(header_path "${CMAKE_SOURCE_DIR}/include/ldp-abst")

# headers to be installed
set(LDP_ABST_PUBLIC_INCLUDE
		${header_path}/datatypes.h
		${header_path}/field.h
		${header_path}/ldp_search_delay.h
		${header_path}/ldpc.h
		${header_path}/VBICompact.h
		${header_path}/VBIMini.h
		)

# source files to be built
set(LDP_ABST_SRCS
		ldp_search_delay.c
		ldpc.c
		VBICompact.c
)

add_library(ldp_abst ${LDP_ABST_PUBLIC_INCLUDE} ${LDP_ABST_SRCS} )

# So that anything that links to our lib gets the headers for all dependencies
target_include_directories(ldp_abst PUBLIC

		# this will be populated when installing but blank when building
		# for client in install mode
		$<INSTALL_INTERFACE:${include_dest}>

		# this will be populated when building but blank when installing
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

install(FILES ${LDP_ABST_PUBLIC_INCLUDE} DESTINATION "${include_dest}/ldp-abst")

# goes into lib_dest so that Debug/Release is taken into account
install(TARGETS ldp_abst EXPORT ldp_abst DESTINATION "${lib_dest}")

#########################################################################################

# AVR builds will not want this (it probably won't even build)
# the unit tests rely on this library being build so we'd have to decouple that if we are going to use a different variable
if (BUILD_TESTING)

	# This library is separate because it's written in C++ and the AVR builds can only consume C libraries.
  add_library(ldp_abst_util ${header_path}/VideoStandard.h VBIParse.cpp ${header_path}/VBIParse.h)

  # So that anything that links to our lib gets the headers for all dependencies
  target_include_directories(ldp_abst_util PUBLIC

		# this will be populated when installing but blank when building
		# for client in install mode
		$<INSTALL_INTERFACE:${include_dest}>

		# this will be populated when building but blank when installing
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

  # for mutation testing (we don't want to change all CXX flags because we don't want to mutate test runners/frameworks)
  if(DEFINED LDP_ABST_UTIL_COMPILE_OPTIONS)
    target_compile_options(ldp_abst_util PRIVATE SHELL:${LDP_ABST_UTIL_COMPILE_OPTIONS})
  endif()

endif()