project(tests)

find_package(Qt5Test REQUIRED)

SET(CMAKE_AUTOMOC ON)

SET(MY_TEST_LIBS Qt5::Test )

ENABLE_TESTING()


add_executable(StoreOperationsTest storeoperationstest.cpp)
add_test(NAME StoreOperationsTest COMMAND StoreOperationsTest)
target_link_libraries(StoreOperationsTest PRIVATE ${MY_TEST_LIBS} )

