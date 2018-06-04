file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(TestDeviceTests ${SRCS})

TARGET_LINK_LIBRARIES(
    TestDeviceTests
    libgtest
    libgmock
    stdc++fs
)

add_test(NAME TestDeviceTests
         COMMAND TestDeviceTests)