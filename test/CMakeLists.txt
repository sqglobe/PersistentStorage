project(tests)

find_package(Qt5Test REQUIRED)
find_package(BerkeleyDB REQUIRED)

SET(CMAKE_AUTOMOC ON)

SET(MY_TEST_LIBS Qt5::Test ${LIBRARY_NAME})

ENABLE_TESTING()


add_executable(StoreOperationsTest storeoperationstest.cpp)
add_test(NAME StoreOperationsTest COMMAND StoreOperationsTest)
target_link_libraries(StoreOperationsTest PRIVATE ${MY_TEST_LIBS})
target_include_directories( StoreOperationsTest PRIVATE ${CMAKE_SOURCE_DIR}/submodules/eventpp/include)


add_executable(ChildStorageTest childstoragetest.cpp)
add_test(NAME ChildStorageTest COMMAND ChildStorageTest)
target_link_libraries(ChildStorageTest PRIVATE ${MY_TEST_LIBS})
target_include_directories( ChildStorageTest PRIVATE ${CMAKE_SOURCE_DIR}/submodules/eventpp/include)

add_executable(ChangeWatcherTest changewatchertest.cpp)
add_test(NAME ChangeWatcherTest COMMAND ChangeWatcherTest)
target_link_libraries(ChangeWatcherTest PRIVATE ${MY_TEST_LIBS})
target_include_directories( ChangeWatcherTest PRIVATE ${CMAKE_SOURCE_DIR}/submodules/eventpp/include)

add_executable(StoreWithWatcherTest storewithwatchertest.cpp)
add_test(NAME StoreWithWatcherTest COMMAND StoreWithWatcherTest)
target_link_libraries(StoreWithWatcherTest PRIVATE ${MY_TEST_LIBS})
target_include_directories( StoreWithWatcherTest PRIVATE ${CMAKE_SOURCE_DIR}/submodules/eventpp/include)
