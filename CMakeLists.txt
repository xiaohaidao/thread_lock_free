cmake_minimum_required(VERSION 3.14)

project(thread_lock_free
    VERSION 1.0.1
    )

set(CMAKE_CXX_STANDARD 11)
ADD_SUBDIRECTORY(src)

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
      googletest_v1.11
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG release-1.11.0
    )
    FetchContent_MakeAvailable(googletest_v1.11)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(tests)

endif()
