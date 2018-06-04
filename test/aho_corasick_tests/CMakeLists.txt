file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(AhoCorasickTest ${SRCS})

TARGET_LINK_LIBRARIES(
    AhoCorasickTest
    libgtest
)

add_test(NAME AhoCorasickTest
         COMMAND AhoCorasickTest)