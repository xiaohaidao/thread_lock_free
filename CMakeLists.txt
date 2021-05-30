cmake_minimum_required(VERSION 3.1)

project(thread_lock_free
    VERSION 1.0.1
    )

set(CMAKE_CXX_STANDARD 11)
ADD_SUBDIRECTORY(src)

include(CTest)
if(BUILD_TESTING)
    ADD_SUBDIRECTORY(thirdparty/googletest)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(tests)
endif()
