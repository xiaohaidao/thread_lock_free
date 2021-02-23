
include_directories(${PROJECT_SOURCE_DIR}/src
    )

add_executable(lock_free_stack_test1
    lock_free_stack_test1.cpp
    )

add_test(NAME lock_free_stack_test1
    COMMAND $<TARGET_FILE:lock_free_stack_test1>)

add_executable(lock_free_queue_test1
    lock_free_queue_test1.cpp
    )

add_test(NAME lock_free_queue_test1
    COMMAND $<TARGET_FILE:lock_free_queue_test1>)
