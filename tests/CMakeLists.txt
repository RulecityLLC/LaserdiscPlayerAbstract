
set(TEST_LDP_ABST_SRCS
        vbi_test.cpp
		ldpc_tests.cpp
        stdafx.h
		VideoStandard.h
		VBIParse.cpp
		VBIParse.h
		mpo_deleter.h
)

add_executable(test_ldp_abst ${TEST_LDP_ABST_SRCS})

# should help improve compile time for test project
target_precompile_headers(test_ldp_abst PRIVATE stdafx.h)

# this will automatically give the indicated targets access to the headers/libs of the indicated dependencies
target_link_libraries(test_ldp_abst LINK_PUBLIC ldp_abst gmock gtest_main)
