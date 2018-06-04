file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(StorageTests ${SRCS})

TARGET_LINK_LIBRARIES(
    StorageTests
    libgtest
    libgmock
    stdc++fs
)

add_test(NAME StorageTests
         COMMAND StorageTests)