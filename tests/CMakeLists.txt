
include_directories(${PROJECT_SOURCE_DIR}/src
    )

add_executable(lock_free_test1
    lock_free_queue_test1.cpp
    lock_free_stack_test1.cpp
    interrupt_thread_test1.cpp
    main.cpp
    )

target_link_libraries(lock_free_test1 gtest_main)
if(UNIX OR MINGW)
target_link_libraries(lock_free_test1 atomic pthread)
endif()

add_test(NAME lock_free_test1
    COMMAND $<TARGET_FILE:lock_free_test1>)
