file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(VersionTests ${SRCS})

TARGET_LINK_LIBRARIES(
    VersionTests
    libgtest
    libgmock
)

add_test(NAME VersionTests
         COMMAND VersionTests)