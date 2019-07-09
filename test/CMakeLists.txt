project(tests)

find_package(Qt5Test REQUIRED)

SET(CMAKE_AUTOMOC ON)

SET(MY_TEST_LIBS Qt5::Test)

ENABLE_TESTING()

target_link_libraries(PersistentStorage)

add_executable(StoreOperationsTest storeoperationstest.cpp)
add_test(NAME StoreOperationsTest COMMAND StoreOperationsTest)
target_link_libraries(StoreOperationsTest PRIVATE ${MY_TEST_LIBS} PersistentStorage)

add_executable(ChildStorageTest childstoragetest.cpp)
add_test(NAME ChildStorageTest COMMAND ChildStorageTest)
target_link_libraries(ChildStorageTest PRIVATE ${MY_TEST_LIBS} PersistentStorage)

