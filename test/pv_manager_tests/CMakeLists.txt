file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(PVManagerTests ${SRCS})

TARGET_LINK_LIBRARIES(
    PVManagerTests
    libgtest
    libgmock
    stdc++fs
)

add_test(NAME PVManagerTests
         COMMAND PVManagerTests)