project(tests)

find_package(Qt5Test REQUIRED)

SET(MY_TEST_LIBS Qt5::Test )

ENABLE_TESTING()


#add_executable(DialogMessageCreationFromsourceTest dialogmessagecreationfromsourcetest.cpp)
#add_test(NAME DialogMessageCreationFromsourceTest COMMAND DialogMessageCreationFromsourceTest)
#target_link_libraries(DialogMessageCreationFromsourceTest PRIVATE ${MY_TEST_LIBS} )

