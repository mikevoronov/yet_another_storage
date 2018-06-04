file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(TimeTests ${SRCS})

TARGET_LINK_LIBRARIES(
    TimeTests
    libgtest
    libgmock
)

add_test(NAME TimeTests
         COMMAND TimeTests)