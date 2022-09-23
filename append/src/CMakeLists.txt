SET(SRC_LIST main.cpp)
MESSAGE(STAUTS  "this is a binary dir: ", ${APPEND_BINARY_DIR})
MESSAGE(STAUTS  "this is a source dir: ", ${APPEND_SOURCE_DIR})
ADD_EXECUTABLE(append ${SRC_LIST})

