
set(TEST_LDP_ABST_SRCS
		VBICompactTest.cpp
		LDPCTest.cpp
        stdafx.h
		VBIParseTest.cpp
		mpo_deleter.h
)

# we want all test helper files to be in the same dir as the test exe
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_executable(test_ldp_abst ${TEST_LDP_ABST_SRCS})

# copy resources needed for tests (see https://stackoverflow.com/questions/24311402/copy-all-files-with-given-extension-to-output-directory-using-cmake )
file(GLOB MY_TEST_HELPERS
  ${CMAKE_SOURCE_DIR}/tests/test_data_files/*.bin
)
file(COPY ${MY_TEST_HELPERS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# should help improve compile time for test project
target_precompile_headers(test_ldp_abst PRIVATE stdafx.h)

# this will automatically give the indicated targets access to the headers/libs of the indicated dependencies
target_link_libraries(test_ldp_abst LINK_PUBLIC ldp_abst ldp_abst_util gmock gtest_main)
