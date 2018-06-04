file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(FreelistHelperTests ${SRCS})

TARGET_LINK_LIBRARIES(
    FreelistHelperTests
    libgtest
    libgmock
)

add_test(NAME FreelistHelperTests
         COMMAND FreelistHelperTests)