file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(InvertedIndexTests ${SRCS})

TARGET_LINK_LIBRARIES(
    InvertedIndexTests
    libgtest
    libgmock
)

add_test(NAME InvertedIndexTests
         COMMAND InvertedIndexTests)